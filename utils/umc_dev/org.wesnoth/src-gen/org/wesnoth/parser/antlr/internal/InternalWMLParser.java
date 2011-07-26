package org.wesnoth.parser.antlr.internal; 

import org.eclipse.xtext.*;
import org.eclipse.xtext.parser.*;
import org.eclipse.xtext.parser.impl.*;
import org.eclipse.emf.ecore.util.EcoreUtil;
import org.eclipse.emf.ecore.EObject;
import org.eclipse.xtext.parser.antlr.AbstractInternalAntlrParser;
import org.eclipse.xtext.parser.antlr.XtextTokenStream;
import org.eclipse.xtext.parser.antlr.XtextTokenStream.HiddenTokens;
import org.eclipse.xtext.parser.antlr.AntlrDatatypeRuleToken;
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
    public static final int RULE_IFDEF=9;
    public static final int RULE_ID=4;
    public static final int T__29=29;
    public static final int T__28=28;
    public static final int T__27=27;
    public static final int T__26=26;
    public static final int T__25=25;
    public static final int T__24=24;
    public static final int T__23=23;
    public static final int T__22=22;
    public static final int RULE_ANY_OTHER=18;
    public static final int T__21=21;
    public static final int T__20=20;
    public static final int RULE_IFNDEF=10;
    public static final int RULE_EOL=5;
    public static final int RULE_TEXTDOMAIN=15;
    public static final int RULE_IFNHAVE=12;
    public static final int RULE_SL_COMMENT=6;
    public static final int EOF=-1;
    public static final int T__30=30;
    public static final int T__31=31;
    public static final int RULE_STRING=17;
    public static final int T__32=32;
    public static final int T__33=33;
    public static final int RULE_ENDIF=14;
    public static final int T__34=34;
    public static final int RULE_DEFINE=7;
    public static final int RULE_ENDDEF=8;
    public static final int RULE_IFHAVE=11;
    public static final int RULE_WS=19;
    public static final int RULE_ELSE=13;

    // delegates
    // delegators


        public InternalWMLParser(TokenStream input) {
            this(input, new RecognizerSharedState());
        }
        public InternalWMLParser(TokenStream input, RecognizerSharedState state) {
            super(input, state);
             
        }
        

    public String[] getTokenNames() { return InternalWMLParser.tokenNames; }
    public String getGrammarFileName() { return "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g"; }



     	private WMLGrammarAccess grammarAccess;
     	
        public InternalWMLParser(TokenStream input, WMLGrammarAccess grammarAccess) {
            this(input);
            this.grammarAccess = grammarAccess;
            registerRules(grammarAccess.getGrammar());
        }
        
        @Override
        protected String getFirstRuleName() {
        	return "WMLRoot";	
       	}
       	
       	@Override
       	protected WMLGrammarAccess getGrammarAccess() {
       		return grammarAccess;
       	}



    // $ANTLR start "entryRuleWMLRoot"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:67:1: entryRuleWMLRoot returns [EObject current=null] : iv_ruleWMLRoot= ruleWMLRoot EOF ;
    public final EObject entryRuleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLRoot = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:68:2: (iv_ruleWMLRoot= ruleWMLRoot EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:69:2: iv_ruleWMLRoot= ruleWMLRoot EOF
            {
             newCompositeNode(grammarAccess.getWMLRootRule()); 
            pushFollow(FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75);
            iv_ruleWMLRoot=ruleWMLRoot();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLRoot"


    // $ANTLR start "ruleWMLRoot"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:76:1: ruleWMLRoot returns [EObject current=null] : ( (lv_Expressions_0_0= ruleWMLRootExpression ) )* ;
    public final EObject ruleWMLRoot() throws RecognitionException {
        EObject current = null;

        EObject lv_Expressions_0_0 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:79:28: ( ( (lv_Expressions_0_0= ruleWMLRootExpression ) )* )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:80:1: ( (lv_Expressions_0_0= ruleWMLRootExpression ) )*
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:80:1: ( (lv_Expressions_0_0= ruleWMLRootExpression ) )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0==RULE_DEFINE||(LA1_0>=RULE_IFDEF && LA1_0<=RULE_IFNHAVE)||LA1_0==RULE_TEXTDOMAIN||LA1_0==20||LA1_0==25) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:81:1: (lv_Expressions_0_0= ruleWMLRootExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:81:1: (lv_Expressions_0_0= ruleWMLRootExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:82:3: lv_Expressions_0_0= ruleWMLRootExpression
            	    {
            	     
            	    	        newCompositeNode(grammarAccess.getWMLRootAccess().getExpressionsWMLRootExpressionParserRuleCall_0()); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLRootExpression_in_ruleWMLRoot130);
            	    lv_Expressions_0_0=ruleWMLRootExpression();

            	    state._fsp--;


            	    	        if (current==null) {
            	    	            current = createModelElementForParent(grammarAccess.getWMLRootRule());
            	    	        }
            	           		add(
            	           			current, 
            	           			"Expressions",
            	            		lv_Expressions_0_0, 
            	            		"WMLRootExpression");
            	    	        afterParserOrEnumRuleCall();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLRoot"


    // $ANTLR start "entryRuleWMLTag"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:106:1: entryRuleWMLTag returns [EObject current=null] : iv_ruleWMLTag= ruleWMLTag EOF ;
    public final EObject entryRuleWMLTag() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTag = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:107:2: (iv_ruleWMLTag= ruleWMLTag EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:108:2: iv_ruleWMLTag= ruleWMLTag EOF
            {
             newCompositeNode(grammarAccess.getWMLTagRule()); 
            pushFollow(FOLLOW_ruleWMLTag_in_entryRuleWMLTag166);
            iv_ruleWMLTag=ruleWMLTag();

            state._fsp--;

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
    // $ANTLR end "entryRuleWMLTag"


    // $ANTLR start "ruleWMLTag"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:115:1: ruleWMLTag returns [EObject current=null] : (otherlv_0= '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) otherlv_3= ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* otherlv_5= '[/' ( (lv_endName_6_0= RULE_ID ) ) otherlv_7= ']' ) ;
    public final EObject ruleWMLTag() throws RecognitionException {
        EObject current = null;

        Token otherlv_0=null;
        Token lv_plus_1_0=null;
        Token lv_name_2_0=null;
        Token otherlv_3=null;
        Token otherlv_5=null;
        Token lv_endName_6_0=null;
        Token otherlv_7=null;
        EObject lv_Expressions_4_0 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:118:28: ( (otherlv_0= '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) otherlv_3= ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* otherlv_5= '[/' ( (lv_endName_6_0= RULE_ID ) ) otherlv_7= ']' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:119:1: (otherlv_0= '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) otherlv_3= ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* otherlv_5= '[/' ( (lv_endName_6_0= RULE_ID ) ) otherlv_7= ']' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:119:1: (otherlv_0= '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) otherlv_3= ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* otherlv_5= '[/' ( (lv_endName_6_0= RULE_ID ) ) otherlv_7= ']' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:119:3: otherlv_0= '[' ( (lv_plus_1_0= '+' ) )? ( (lv_name_2_0= RULE_ID ) ) otherlv_3= ']' ( (lv_Expressions_4_0= ruleWMLExpression ) )* otherlv_5= '[/' ( (lv_endName_6_0= RULE_ID ) ) otherlv_7= ']'
            {
            otherlv_0=(Token)match(input,20,FOLLOW_20_in_ruleWMLTag213); 

                	newLeafNode(otherlv_0, grammarAccess.getWMLTagAccess().getLeftSquareBracketKeyword_0());
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:123:1: ( (lv_plus_1_0= '+' ) )?
            int alt2=2;
            int LA2_0 = input.LA(1);

            if ( (LA2_0==21) ) {
                alt2=1;
            }
            switch (alt2) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:124:1: (lv_plus_1_0= '+' )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:124:1: (lv_plus_1_0= '+' )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:125:3: lv_plus_1_0= '+'
                    {
                    lv_plus_1_0=(Token)match(input,21,FOLLOW_21_in_ruleWMLTag231); 

                            newLeafNode(lv_plus_1_0, grammarAccess.getWMLTagAccess().getPlusPlusSignKeyword_1_0());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLTagRule());
                    	        }
                           		setWithLastConsumed(current, "plus", lv_plus_1_0, "+");
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:138:3: ( (lv_name_2_0= RULE_ID ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:139:1: (lv_name_2_0= RULE_ID )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:139:1: (lv_name_2_0= RULE_ID )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:140:3: lv_name_2_0= RULE_ID
            {
            lv_name_2_0=(Token)match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag262); 

            			newLeafNode(lv_name_2_0, grammarAccess.getWMLTagAccess().getNameIDTerminalRuleCall_2_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLTagRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"name",
                    		lv_name_2_0, 
                    		"ID");
            	    

            }


            }

            otherlv_3=(Token)match(input,22,FOLLOW_22_in_ruleWMLTag279); 

                	newLeafNode(otherlv_3, grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_3());
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:160:1: ( (lv_Expressions_4_0= ruleWMLExpression ) )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( (LA3_0==RULE_ID||LA3_0==RULE_DEFINE||(LA3_0>=RULE_IFDEF && LA3_0<=RULE_IFNHAVE)||LA3_0==RULE_TEXTDOMAIN||LA3_0==20||LA3_0==25) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:161:1: (lv_Expressions_4_0= ruleWMLExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:161:1: (lv_Expressions_4_0= ruleWMLExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:162:3: lv_Expressions_4_0= ruleWMLExpression
            	    {
            	     
            	    	        newCompositeNode(grammarAccess.getWMLTagAccess().getExpressionsWMLExpressionParserRuleCall_4_0()); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLExpression_in_ruleWMLTag300);
            	    lv_Expressions_4_0=ruleWMLExpression();

            	    state._fsp--;


            	    	        if (current==null) {
            	    	            current = createModelElementForParent(grammarAccess.getWMLTagRule());
            	    	        }
            	           		add(
            	           			current, 
            	           			"Expressions",
            	            		lv_Expressions_4_0, 
            	            		"WMLExpression");
            	    	        afterParserOrEnumRuleCall();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);

            otherlv_5=(Token)match(input,23,FOLLOW_23_in_ruleWMLTag313); 

                	newLeafNode(otherlv_5, grammarAccess.getWMLTagAccess().getLeftSquareBracketSolidusKeyword_5());
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:182:1: ( (lv_endName_6_0= RULE_ID ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:183:1: (lv_endName_6_0= RULE_ID )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:183:1: (lv_endName_6_0= RULE_ID )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:184:3: lv_endName_6_0= RULE_ID
            {
            lv_endName_6_0=(Token)match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLTag330); 

            			newLeafNode(lv_endName_6_0, grammarAccess.getWMLTagAccess().getEndNameIDTerminalRuleCall_6_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLTagRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"endName",
                    		lv_endName_6_0, 
                    		"ID");
            	    

            }


            }

            otherlv_7=(Token)match(input,22,FOLLOW_22_in_ruleWMLTag347); 

                	newLeafNode(otherlv_7, grammarAccess.getWMLTagAccess().getRightSquareBracketKeyword_7());
                

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLTag"


    // $ANTLR start "entryRuleWMLKey"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:212:1: entryRuleWMLKey returns [EObject current=null] : iv_ruleWMLKey= ruleWMLKey EOF ;
    public final EObject entryRuleWMLKey() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKey = null;


         
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
        	
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:216:2: (iv_ruleWMLKey= ruleWMLKey EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:217:2: iv_ruleWMLKey= ruleWMLKey EOF
            {
             newCompositeNode(grammarAccess.getWMLKeyRule()); 
            pushFollow(FOLLOW_ruleWMLKey_in_entryRuleWMLKey389);
            iv_ruleWMLKey=ruleWMLKey();

            state._fsp--;

             current =iv_ruleWMLKey; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKey399); 

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
    // $ANTLR end "entryRuleWMLKey"


    // $ANTLR start "ruleWMLKey"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:227:1: ruleWMLKey returns [EObject current=null] : ( ( (lv_name_0_0= RULE_ID ) ) otherlv_1= '=' ( (lv_values_2_0= ruleWMLKeyValue ) )* ( (this_EOL_3= RULE_EOL )? otherlv_4= '+' (this_EOL_5= RULE_EOL )? ( (lv_values_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) ) ) ;
    public final EObject ruleWMLKey() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        Token otherlv_1=null;
        Token this_EOL_3=null;
        Token otherlv_4=null;
        Token this_EOL_5=null;
        Token lv_eol_7_1=null;
        Token lv_eol_7_2=null;
        EObject lv_values_2_0 = null;

        EObject lv_values_6_0 = null;


         enterRule(); 
        		HiddenTokens myHiddenTokenState = ((XtextTokenStream)input).setHiddenTokens("RULE_WS");
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:231:28: ( ( ( (lv_name_0_0= RULE_ID ) ) otherlv_1= '=' ( (lv_values_2_0= ruleWMLKeyValue ) )* ( (this_EOL_3= RULE_EOL )? otherlv_4= '+' (this_EOL_5= RULE_EOL )? ( (lv_values_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:232:1: ( ( (lv_name_0_0= RULE_ID ) ) otherlv_1= '=' ( (lv_values_2_0= ruleWMLKeyValue ) )* ( (this_EOL_3= RULE_EOL )? otherlv_4= '+' (this_EOL_5= RULE_EOL )? ( (lv_values_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:232:1: ( ( (lv_name_0_0= RULE_ID ) ) otherlv_1= '=' ( (lv_values_2_0= ruleWMLKeyValue ) )* ( (this_EOL_3= RULE_EOL )? otherlv_4= '+' (this_EOL_5= RULE_EOL )? ( (lv_values_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:232:2: ( (lv_name_0_0= RULE_ID ) ) otherlv_1= '=' ( (lv_values_2_0= ruleWMLKeyValue ) )* ( (this_EOL_3= RULE_EOL )? otherlv_4= '+' (this_EOL_5= RULE_EOL )? ( (lv_values_6_0= ruleWMLKeyValue ) )+ )* ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:232:2: ( (lv_name_0_0= RULE_ID ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:233:1: (lv_name_0_0= RULE_ID )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:233:1: (lv_name_0_0= RULE_ID )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:234:3: lv_name_0_0= RULE_ID
            {
            lv_name_0_0=(Token)match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLKey445); 

            			newLeafNode(lv_name_0_0, grammarAccess.getWMLKeyAccess().getNameIDTerminalRuleCall_0_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLKeyRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"name",
                    		lv_name_0_0, 
                    		"ID");
            	    

            }


            }

            otherlv_1=(Token)match(input,24,FOLLOW_24_in_ruleWMLKey462); 

                	newLeafNode(otherlv_1, grammarAccess.getWMLKeyAccess().getEqualsSignKeyword_1());
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:254:1: ( (lv_values_2_0= ruleWMLKeyValue ) )*
            loop4:
            do {
                int alt4=2;
                int LA4_0 = input.LA(1);

                if ( (LA4_0==RULE_ID||(LA4_0>=RULE_LUA_CODE && LA4_0<=RULE_ANY_OTHER)||LA4_0==20||(LA4_0>=25 && LA4_0<=27)||(LA4_0>=29 && LA4_0<=34)) ) {
                    alt4=1;
                }


                switch (alt4) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:255:1: (lv_values_2_0= ruleWMLKeyValue )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:255:1: (lv_values_2_0= ruleWMLKeyValue )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:256:3: lv_values_2_0= ruleWMLKeyValue
            	    {
            	     
            	    	        newCompositeNode(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_2_0()); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey483);
            	    lv_values_2_0=ruleWMLKeyValue();

            	    state._fsp--;


            	    	        if (current==null) {
            	    	            current = createModelElementForParent(grammarAccess.getWMLKeyRule());
            	    	        }
            	           		add(
            	           			current, 
            	           			"values",
            	            		lv_values_2_0, 
            	            		"WMLKeyValue");
            	    	        afterParserOrEnumRuleCall();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop4;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:272:3: ( (this_EOL_3= RULE_EOL )? otherlv_4= '+' (this_EOL_5= RULE_EOL )? ( (lv_values_6_0= ruleWMLKeyValue ) )+ )*
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:272:4: (this_EOL_3= RULE_EOL )? otherlv_4= '+' (this_EOL_5= RULE_EOL )? ( (lv_values_6_0= ruleWMLKeyValue ) )+
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:272:4: (this_EOL_3= RULE_EOL )?
            	    int alt5=2;
            	    int LA5_0 = input.LA(1);

            	    if ( (LA5_0==RULE_EOL) ) {
            	        alt5=1;
            	    }
            	    switch (alt5) {
            	        case 1 :
            	            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:272:5: this_EOL_3= RULE_EOL
            	            {
            	            this_EOL_3=(Token)match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey497); 
            	             
            	                newLeafNode(this_EOL_3, grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_0()); 
            	                

            	            }
            	            break;

            	    }

            	    otherlv_4=(Token)match(input,21,FOLLOW_21_in_ruleWMLKey510); 

            	        	newLeafNode(otherlv_4, grammarAccess.getWMLKeyAccess().getPlusSignKeyword_3_1());
            	        
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:280:1: (this_EOL_5= RULE_EOL )?
            	    int alt6=2;
            	    int LA6_0 = input.LA(1);

            	    if ( (LA6_0==RULE_EOL) ) {
            	        alt6=1;
            	    }
            	    switch (alt6) {
            	        case 1 :
            	            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:280:2: this_EOL_5= RULE_EOL
            	            {
            	            this_EOL_5=(Token)match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey522); 
            	             
            	                newLeafNode(this_EOL_5, grammarAccess.getWMLKeyAccess().getEOLTerminalRuleCall_3_2()); 
            	                

            	            }
            	            break;

            	    }

            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:284:3: ( (lv_values_6_0= ruleWMLKeyValue ) )+
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
            	    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:285:1: (lv_values_6_0= ruleWMLKeyValue )
            	    	    {
            	    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:285:1: (lv_values_6_0= ruleWMLKeyValue )
            	    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:286:3: lv_values_6_0= ruleWMLKeyValue
            	    	    {
            	    	     
            	    	    	        newCompositeNode(grammarAccess.getWMLKeyAccess().getValuesWMLKeyValueParserRuleCall_3_3_0()); 
            	    	    	    
            	    	    pushFollow(FOLLOW_ruleWMLKeyValue_in_ruleWMLKey544);
            	    	    lv_values_6_0=ruleWMLKeyValue();

            	    	    state._fsp--;


            	    	    	        if (current==null) {
            	    	    	            current = createModelElementForParent(grammarAccess.getWMLKeyRule());
            	    	    	        }
            	    	           		add(
            	    	           			current, 
            	    	           			"values",
            	    	            		lv_values_6_0, 
            	    	            		"WMLKeyValue");
            	    	    	        afterParserOrEnumRuleCall();
            	    	    	    

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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:302:5: ( ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:303:1: ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:303:1: ( (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:304:1: (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:304:1: (lv_eol_7_1= RULE_EOL | lv_eol_7_2= RULE_SL_COMMENT )
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
                    new NoViableAltException("", 9, 0, input);

                throw nvae;
            }
            switch (alt9) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:305:3: lv_eol_7_1= RULE_EOL
                    {
                    lv_eol_7_1=(Token)match(input,RULE_EOL,FOLLOW_RULE_EOL_in_ruleWMLKey566); 

                    			newLeafNode(lv_eol_7_1, grammarAccess.getWMLKeyAccess().getEolEOLTerminalRuleCall_4_0_0()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLKeyRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"eol",
                            		lv_eol_7_1, 
                            		"EOL");
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:320:8: lv_eol_7_2= RULE_SL_COMMENT
                    {
                    lv_eol_7_2=(Token)match(input,RULE_SL_COMMENT,FOLLOW_RULE_SL_COMMENT_in_ruleWMLKey586); 

                    			newLeafNode(lv_eol_7_2, grammarAccess.getWMLKeyAccess().getEolSL_COMMENTTerminalRuleCall_4_0_1()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLKeyRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"eol",
                            		lv_eol_7_2, 
                            		"SL_COMMENT");
                    	    

                    }
                    break;

            }


            }


            }


            }


            }

             leaveRule(); 
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
    // $ANTLR end "ruleWMLKey"


    // $ANTLR start "entryRuleWMLKeyValue"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:349:1: entryRuleWMLKeyValue returns [EObject current=null] : iv_ruleWMLKeyValue= ruleWMLKeyValue EOF ;
    public final EObject entryRuleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLKeyValue = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:350:2: (iv_ruleWMLKeyValue= ruleWMLKeyValue EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:351:2: iv_ruleWMLKeyValue= ruleWMLKeyValue EOF
            {
             newCompositeNode(grammarAccess.getWMLKeyValueRule()); 
            pushFollow(FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue634);
            iv_ruleWMLKeyValue=ruleWMLKeyValue();

            state._fsp--;

             current =iv_ruleWMLKeyValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLKeyValue644); 

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
    // $ANTLR end "entryRuleWMLKeyValue"


    // $ANTLR start "ruleWMLKeyValue"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:358:1: ruleWMLKeyValue returns [EObject current=null] : (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall ) ;
    public final EObject ruleWMLKeyValue() throws RecognitionException {
        EObject current = null;

        EObject this_WMLValue_0 = null;

        EObject this_WMLMacroCall_1 = null;

        EObject this_WMLLuaCode_2 = null;

        EObject this_WMLArrayCall_3 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:361:28: ( (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:362:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:362:1: (this_WMLValue_0= ruleWMLValue | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLLuaCode_2= ruleWMLLuaCode | this_WMLArrayCall_3= ruleWMLArrayCall )
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
                    new NoViableAltException("", 10, 0, input);

                throw nvae;
            }

            switch (alt10) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:363:5: this_WMLValue_0= ruleWMLValue
                    {
                     
                            newCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLValueParserRuleCall_0()); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLKeyValue691);
                    this_WMLValue_0=ruleWMLValue();

                    state._fsp--;

                     
                            current = this_WMLValue_0; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:373:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            newCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLMacroCallParserRuleCall_1()); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue718);
                    this_WMLMacroCall_1=ruleWMLMacroCall();

                    state._fsp--;

                     
                            current = this_WMLMacroCall_1; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:383:5: this_WMLLuaCode_2= ruleWMLLuaCode
                    {
                     
                            newCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLLuaCodeParserRuleCall_2()); 
                        
                    pushFollow(FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue745);
                    this_WMLLuaCode_2=ruleWMLLuaCode();

                    state._fsp--;

                     
                            current = this_WMLLuaCode_2; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:393:5: this_WMLArrayCall_3= ruleWMLArrayCall
                    {
                     
                            newCompositeNode(grammarAccess.getWMLKeyValueAccess().getWMLArrayCallParserRuleCall_3()); 
                        
                    pushFollow(FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue772);
                    this_WMLArrayCall_3=ruleWMLArrayCall();

                    state._fsp--;

                     
                            current = this_WMLArrayCall_3; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLKeyValue"


    // $ANTLR start "entryRuleWMLMacroCall"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:409:1: entryRuleWMLMacroCall returns [EObject current=null] : iv_ruleWMLMacroCall= ruleWMLMacroCall EOF ;
    public final EObject entryRuleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCall = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:410:2: (iv_ruleWMLMacroCall= ruleWMLMacroCall EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:411:2: iv_ruleWMLMacroCall= ruleWMLMacroCall EOF
            {
             newCompositeNode(grammarAccess.getWMLMacroCallRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall807);
            iv_ruleWMLMacroCall=ruleWMLMacroCall();

            state._fsp--;

             current =iv_ruleWMLMacroCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCall817); 

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
    // $ANTLR end "entryRuleWMLMacroCall"


    // $ANTLR start "ruleWMLMacroCall"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:418:1: ruleWMLMacroCall returns [EObject current=null] : (otherlv_0= '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* otherlv_5= '}' ) ;
    public final EObject ruleWMLMacroCall() throws RecognitionException {
        EObject current = null;

        Token otherlv_0=null;
        Token lv_point_1_0=null;
        Token lv_relative_2_0=null;
        Token lv_name_3_0=null;
        Token otherlv_5=null;
        EObject lv_Parameters_4_0 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:421:28: ( (otherlv_0= '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* otherlv_5= '}' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:422:1: (otherlv_0= '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* otherlv_5= '}' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:422:1: (otherlv_0= '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* otherlv_5= '}' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:422:3: otherlv_0= '{' ( (lv_point_1_0= './' ) )? ( (lv_relative_2_0= '~' ) )? ( (lv_name_3_0= RULE_ID ) ) ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )* otherlv_5= '}'
            {
            otherlv_0=(Token)match(input,25,FOLLOW_25_in_ruleWMLMacroCall854); 

                	newLeafNode(otherlv_0, grammarAccess.getWMLMacroCallAccess().getLeftCurlyBracketKeyword_0());
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:426:1: ( (lv_point_1_0= './' ) )?
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0==26) ) {
                alt11=1;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:427:1: (lv_point_1_0= './' )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:427:1: (lv_point_1_0= './' )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:428:3: lv_point_1_0= './'
                    {
                    lv_point_1_0=(Token)match(input,26,FOLLOW_26_in_ruleWMLMacroCall872); 

                            newLeafNode(lv_point_1_0, grammarAccess.getWMLMacroCallAccess().getPointFullStopSolidusKeyword_1_0());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLMacroCallRule());
                    	        }
                           		setWithLastConsumed(current, "point", lv_point_1_0, "./");
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:441:3: ( (lv_relative_2_0= '~' ) )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0==27) ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:442:1: (lv_relative_2_0= '~' )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:442:1: (lv_relative_2_0= '~' )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:443:3: lv_relative_2_0= '~'
                    {
                    lv_relative_2_0=(Token)match(input,27,FOLLOW_27_in_ruleWMLMacroCall904); 

                            newLeafNode(lv_relative_2_0, grammarAccess.getWMLMacroCallAccess().getRelativeTildeKeyword_2_0());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLMacroCallRule());
                    	        }
                           		setWithLastConsumed(current, "relative", lv_relative_2_0, "~");
                    	    

                    }


                    }
                    break;

            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:456:3: ( (lv_name_3_0= RULE_ID ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:457:1: (lv_name_3_0= RULE_ID )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:457:1: (lv_name_3_0= RULE_ID )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:458:3: lv_name_3_0= RULE_ID
            {
            lv_name_3_0=(Token)match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLMacroCall935); 

            			newLeafNode(lv_name_3_0, grammarAccess.getWMLMacroCallAccess().getNameIDTerminalRuleCall_3_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLMacroCallRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"name",
                    		lv_name_3_0, 
                    		"ID");
            	    

            }


            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:474:2: ( (lv_Parameters_4_0= ruleWMLMacroCallParameter ) )*
            loop13:
            do {
                int alt13=2;
                int LA13_0 = input.LA(1);

                if ( (LA13_0==RULE_ID||(LA13_0>=RULE_STRING && LA13_0<=RULE_ANY_OTHER)||(LA13_0>=20 && LA13_0<=27)||(LA13_0>=29 && LA13_0<=34)) ) {
                    alt13=1;
                }


                switch (alt13) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:475:1: (lv_Parameters_4_0= ruleWMLMacroCallParameter )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:475:1: (lv_Parameters_4_0= ruleWMLMacroCallParameter )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:476:3: lv_Parameters_4_0= ruleWMLMacroCallParameter
            	    {
            	     
            	    	        newCompositeNode(grammarAccess.getWMLMacroCallAccess().getParametersWMLMacroCallParameterParserRuleCall_4_0()); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_ruleWMLMacroCall961);
            	    lv_Parameters_4_0=ruleWMLMacroCallParameter();

            	    state._fsp--;


            	    	        if (current==null) {
            	    	            current = createModelElementForParent(grammarAccess.getWMLMacroCallRule());
            	    	        }
            	           		add(
            	           			current, 
            	           			"Parameters",
            	            		lv_Parameters_4_0, 
            	            		"WMLMacroCallParameter");
            	    	        afterParserOrEnumRuleCall();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop13;
                }
            } while (true);

            otherlv_5=(Token)match(input,28,FOLLOW_28_in_ruleWMLMacroCall974); 

                	newLeafNode(otherlv_5, grammarAccess.getWMLMacroCallAccess().getRightCurlyBracketKeyword_5());
                

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLMacroCall"


    // $ANTLR start "entryRuleWMLMacroCallParameter"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:504:1: entryRuleWMLMacroCallParameter returns [EObject current=null] : iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF ;
    public final EObject entryRuleWMLMacroCallParameter() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroCallParameter = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:505:2: (iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:506:2: iv_ruleWMLMacroCallParameter= ruleWMLMacroCallParameter EOF
            {
             newCompositeNode(grammarAccess.getWMLMacroCallParameterRule()); 
            pushFollow(FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter1010);
            iv_ruleWMLMacroCallParameter=ruleWMLMacroCallParameter();

            state._fsp--;

             current =iv_ruleWMLMacroCallParameter; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroCallParameter1020); 

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
    // $ANTLR end "entryRuleWMLMacroCallParameter"


    // $ANTLR start "ruleWMLMacroCallParameter"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:513:1: ruleWMLMacroCallParameter returns [EObject current=null] : (this_WMLMacroParameter_0= ruleWMLMacroParameter | this_WMLMacroCall_1= ruleWMLMacroCall ) ;
    public final EObject ruleWMLMacroCallParameter() throws RecognitionException {
        EObject current = null;

        EObject this_WMLMacroParameter_0 = null;

        EObject this_WMLMacroCall_1 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:516:28: ( (this_WMLMacroParameter_0= ruleWMLMacroParameter | this_WMLMacroCall_1= ruleWMLMacroCall ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:517:1: (this_WMLMacroParameter_0= ruleWMLMacroParameter | this_WMLMacroCall_1= ruleWMLMacroCall )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:517:1: (this_WMLMacroParameter_0= ruleWMLMacroParameter | this_WMLMacroCall_1= ruleWMLMacroCall )
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
                    new NoViableAltException("", 14, 0, input);

                throw nvae;
            }
            switch (alt14) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:518:5: this_WMLMacroParameter_0= ruleWMLMacroParameter
                    {
                     
                            newCompositeNode(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroParameterParserRuleCall_0()); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroParameter_in_ruleWMLMacroCallParameter1067);
                    this_WMLMacroParameter_0=ruleWMLMacroParameter();

                    state._fsp--;

                     
                            current = this_WMLMacroParameter_0; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:528:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            newCompositeNode(grammarAccess.getWMLMacroCallParameterAccess().getWMLMacroCallParserRuleCall_1()); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCallParameter1094);
                    this_WMLMacroCall_1=ruleWMLMacroCall();

                    state._fsp--;

                     
                            current = this_WMLMacroCall_1; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLMacroCallParameter"


    // $ANTLR start "entryRuleWMLArrayCall"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:544:1: entryRuleWMLArrayCall returns [EObject current=null] : iv_ruleWMLArrayCall= ruleWMLArrayCall EOF ;
    public final EObject entryRuleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLArrayCall = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:545:2: (iv_ruleWMLArrayCall= ruleWMLArrayCall EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:546:2: iv_ruleWMLArrayCall= ruleWMLArrayCall EOF
            {
             newCompositeNode(grammarAccess.getWMLArrayCallRule()); 
            pushFollow(FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1129);
            iv_ruleWMLArrayCall=ruleWMLArrayCall();

            state._fsp--;

             current =iv_ruleWMLArrayCall; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLArrayCall1139); 

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
    // $ANTLR end "entryRuleWMLArrayCall"


    // $ANTLR start "ruleWMLArrayCall"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:553:1: ruleWMLArrayCall returns [EObject current=null] : (otherlv_0= '[' ( (lv_value_1_0= ruleWMLValue ) )+ otherlv_2= ']' ) ;
    public final EObject ruleWMLArrayCall() throws RecognitionException {
        EObject current = null;

        Token otherlv_0=null;
        Token otherlv_2=null;
        EObject lv_value_1_0 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:556:28: ( (otherlv_0= '[' ( (lv_value_1_0= ruleWMLValue ) )+ otherlv_2= ']' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:557:1: (otherlv_0= '[' ( (lv_value_1_0= ruleWMLValue ) )+ otherlv_2= ']' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:557:1: (otherlv_0= '[' ( (lv_value_1_0= ruleWMLValue ) )+ otherlv_2= ']' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:557:3: otherlv_0= '[' ( (lv_value_1_0= ruleWMLValue ) )+ otherlv_2= ']'
            {
            otherlv_0=(Token)match(input,20,FOLLOW_20_in_ruleWMLArrayCall1176); 

                	newLeafNode(otherlv_0, grammarAccess.getWMLArrayCallAccess().getLeftSquareBracketKeyword_0());
                
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:561:1: ( (lv_value_1_0= ruleWMLValue ) )+
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:562:1: (lv_value_1_0= ruleWMLValue )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:562:1: (lv_value_1_0= ruleWMLValue )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:563:3: lv_value_1_0= ruleWMLValue
            	    {
            	     
            	    	        newCompositeNode(grammarAccess.getWMLArrayCallAccess().getValueWMLValueParserRuleCall_1_0()); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1197);
            	    lv_value_1_0=ruleWMLValue();

            	    state._fsp--;


            	    	        if (current==null) {
            	    	            current = createModelElementForParent(grammarAccess.getWMLArrayCallRule());
            	    	        }
            	           		add(
            	           			current, 
            	           			"value",
            	            		lv_value_1_0, 
            	            		"WMLValue");
            	    	        afterParserOrEnumRuleCall();
            	    	    

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

            otherlv_2=(Token)match(input,22,FOLLOW_22_in_ruleWMLArrayCall1210); 

                	newLeafNode(otherlv_2, grammarAccess.getWMLArrayCallAccess().getRightSquareBracketKeyword_2());
                

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLArrayCall"


    // $ANTLR start "entryRuleWMLMacroDefine"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:591:1: entryRuleWMLMacroDefine returns [EObject current=null] : iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF ;
    public final EObject entryRuleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroDefine = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:592:2: (iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:593:2: iv_ruleWMLMacroDefine= ruleWMLMacroDefine EOF
            {
             newCompositeNode(grammarAccess.getWMLMacroDefineRule()); 
            pushFollow(FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1246);
            iv_ruleWMLMacroDefine=ruleWMLMacroDefine();

            state._fsp--;

             current =iv_ruleWMLMacroDefine; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroDefine1256); 

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
    // $ANTLR end "entryRuleWMLMacroDefine"


    // $ANTLR start "ruleWMLMacroDefine"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:600:1: ruleWMLMacroDefine returns [EObject current=null] : ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) ) ;
    public final EObject ruleWMLMacroDefine() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;
        Token lv_endName_2_0=null;
        EObject lv_Expressions_1_0 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:603:28: ( ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:604:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:604:1: ( ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:604:2: ( (lv_name_0_0= RULE_DEFINE ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( (lv_endName_2_0= RULE_ENDDEF ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:604:2: ( (lv_name_0_0= RULE_DEFINE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:605:1: (lv_name_0_0= RULE_DEFINE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:605:1: (lv_name_0_0= RULE_DEFINE )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:606:3: lv_name_0_0= RULE_DEFINE
            {
            lv_name_0_0=(Token)match(input,RULE_DEFINE,FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1298); 

            			newLeafNode(lv_name_0_0, grammarAccess.getWMLMacroDefineAccess().getNameDEFINETerminalRuleCall_0_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLMacroDefineRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"name",
                    		lv_name_0_0, 
                    		"DEFINE");
            	    

            }


            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:622:2: ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )*
            loop16:
            do {
                int alt16=2;
                int LA16_0 = input.LA(1);

                if ( (LA16_0==RULE_ID||LA16_0==RULE_DEFINE||(LA16_0>=RULE_IFDEF && LA16_0<=RULE_IFNHAVE)||LA16_0==RULE_TEXTDOMAIN||(LA16_0>=RULE_STRING && LA16_0<=RULE_ANY_OTHER)||LA16_0==20||(LA16_0>=25 && LA16_0<=27)||(LA16_0>=29 && LA16_0<=34)) ) {
                    alt16=1;
                }


                switch (alt16) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:623:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:623:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:624:3: lv_Expressions_1_0= ruleWMLValuedExpression
            	    {
            	     
            	    	        newCompositeNode(grammarAccess.getWMLMacroDefineAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValuedExpression_in_ruleWMLMacroDefine1324);
            	    lv_Expressions_1_0=ruleWMLValuedExpression();

            	    state._fsp--;


            	    	        if (current==null) {
            	    	            current = createModelElementForParent(grammarAccess.getWMLMacroDefineRule());
            	    	        }
            	           		add(
            	           			current, 
            	           			"Expressions",
            	            		lv_Expressions_1_0, 
            	            		"WMLValuedExpression");
            	    	        afterParserOrEnumRuleCall();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop16;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:640:3: ( (lv_endName_2_0= RULE_ENDDEF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:641:1: (lv_endName_2_0= RULE_ENDDEF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:641:1: (lv_endName_2_0= RULE_ENDDEF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:642:3: lv_endName_2_0= RULE_ENDDEF
            {
            lv_endName_2_0=(Token)match(input,RULE_ENDDEF,FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1342); 

            			newLeafNode(lv_endName_2_0, grammarAccess.getWMLMacroDefineAccess().getEndNameENDDEFTerminalRuleCall_2_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLMacroDefineRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"endName",
                    		lv_endName_2_0, 
                    		"ENDDEF");
            	    

            }


            }


            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLMacroDefine"


    // $ANTLR start "entryRuleWMLPreprocIF"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:666:1: entryRuleWMLPreprocIF returns [EObject current=null] : iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF ;
    public final EObject entryRuleWMLPreprocIF() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLPreprocIF = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:667:2: (iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:668:2: iv_ruleWMLPreprocIF= ruleWMLPreprocIF EOF
            {
             newCompositeNode(grammarAccess.getWMLPreprocIFRule()); 
            pushFollow(FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF1383);
            iv_ruleWMLPreprocIF=ruleWMLPreprocIF();

            state._fsp--;

             current =iv_ruleWMLPreprocIF; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLPreprocIF1393); 

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
    // $ANTLR end "entryRuleWMLPreprocIF"


    // $ANTLR start "ruleWMLPreprocIF"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:675:1: ruleWMLPreprocIF returns [EObject current=null] : ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) ) ) ;
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


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:678:28: ( ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:679:1: ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:679:1: ( ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:679:2: ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) ) ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )* ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )? ( (lv_endName_4_0= RULE_ENDIF ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:679:2: ( ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:680:1: ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:680:1: ( (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:681:1: (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:681:1: (lv_name_0_1= RULE_IFDEF | lv_name_0_2= RULE_IFNDEF | lv_name_0_3= RULE_IFHAVE | lv_name_0_4= RULE_IFNHAVE )
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
                    new NoViableAltException("", 17, 0, input);

                throw nvae;
            }

            switch (alt17) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:682:3: lv_name_0_1= RULE_IFDEF
                    {
                    lv_name_0_1=(Token)match(input,RULE_IFDEF,FOLLOW_RULE_IFDEF_in_ruleWMLPreprocIF1437); 

                    			newLeafNode(lv_name_0_1, grammarAccess.getWMLPreprocIFAccess().getNameIFDEFTerminalRuleCall_0_0_0()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLPreprocIFRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"name",
                            		lv_name_0_1, 
                            		"IFDEF");
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:697:8: lv_name_0_2= RULE_IFNDEF
                    {
                    lv_name_0_2=(Token)match(input,RULE_IFNDEF,FOLLOW_RULE_IFNDEF_in_ruleWMLPreprocIF1457); 

                    			newLeafNode(lv_name_0_2, grammarAccess.getWMLPreprocIFAccess().getNameIFNDEFTerminalRuleCall_0_0_1()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLPreprocIFRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"name",
                            		lv_name_0_2, 
                            		"IFNDEF");
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:712:8: lv_name_0_3= RULE_IFHAVE
                    {
                    lv_name_0_3=(Token)match(input,RULE_IFHAVE,FOLLOW_RULE_IFHAVE_in_ruleWMLPreprocIF1477); 

                    			newLeafNode(lv_name_0_3, grammarAccess.getWMLPreprocIFAccess().getNameIFHAVETerminalRuleCall_0_0_2()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLPreprocIFRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"name",
                            		lv_name_0_3, 
                            		"IFHAVE");
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:727:8: lv_name_0_4= RULE_IFNHAVE
                    {
                    lv_name_0_4=(Token)match(input,RULE_IFNHAVE,FOLLOW_RULE_IFNHAVE_in_ruleWMLPreprocIF1497); 

                    			newLeafNode(lv_name_0_4, grammarAccess.getWMLPreprocIFAccess().getNameIFNHAVETerminalRuleCall_0_0_3()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLPreprocIFRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"name",
                            		lv_name_0_4, 
                            		"IFNHAVE");
                    	    

                    }
                    break;

            }


            }


            }

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:745:2: ( (lv_Expressions_1_0= ruleWMLValuedExpression ) )*
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( (LA18_0==RULE_ID||LA18_0==RULE_DEFINE||(LA18_0>=RULE_IFDEF && LA18_0<=RULE_IFNHAVE)||LA18_0==RULE_TEXTDOMAIN||(LA18_0>=RULE_STRING && LA18_0<=RULE_ANY_OTHER)||LA18_0==20||(LA18_0>=25 && LA18_0<=27)||(LA18_0>=29 && LA18_0<=34)) ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:746:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    {
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:746:1: (lv_Expressions_1_0= ruleWMLValuedExpression )
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:747:3: lv_Expressions_1_0= ruleWMLValuedExpression
            	    {
            	     
            	    	        newCompositeNode(grammarAccess.getWMLPreprocIFAccess().getExpressionsWMLValuedExpressionParserRuleCall_1_0()); 
            	    	    
            	    pushFollow(FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1526);
            	    lv_Expressions_1_0=ruleWMLValuedExpression();

            	    state._fsp--;


            	    	        if (current==null) {
            	    	            current = createModelElementForParent(grammarAccess.getWMLPreprocIFRule());
            	    	        }
            	           		add(
            	           			current, 
            	           			"Expressions",
            	            		lv_Expressions_1_0, 
            	            		"WMLValuedExpression");
            	    	        afterParserOrEnumRuleCall();
            	    	    

            	    }


            	    }
            	    break;

            	default :
            	    break loop18;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:763:3: ( ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+ )?
            int alt20=2;
            int LA20_0 = input.LA(1);

            if ( (LA20_0==RULE_ELSE) ) {
                alt20=1;
            }
            switch (alt20) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:763:4: ( (lv_Elses_2_0= RULE_ELSE ) ) ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:763:4: ( (lv_Elses_2_0= RULE_ELSE ) )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:764:1: (lv_Elses_2_0= RULE_ELSE )
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:764:1: (lv_Elses_2_0= RULE_ELSE )
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:765:3: lv_Elses_2_0= RULE_ELSE
                    {
                    lv_Elses_2_0=(Token)match(input,RULE_ELSE,FOLLOW_RULE_ELSE_in_ruleWMLPreprocIF1545); 

                    			newLeafNode(lv_Elses_2_0, grammarAccess.getWMLPreprocIFAccess().getElsesELSETerminalRuleCall_2_0_0()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLPreprocIFRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"Elses",
                            		lv_Elses_2_0, 
                            		"ELSE");
                    	    

                    }


                    }

                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:781:2: ( (lv_ElseExpressions_3_0= ruleWMLValuedExpression ) )+
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
                    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:782:1: (lv_ElseExpressions_3_0= ruleWMLValuedExpression )
                    	    {
                    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:782:1: (lv_ElseExpressions_3_0= ruleWMLValuedExpression )
                    	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:783:3: lv_ElseExpressions_3_0= ruleWMLValuedExpression
                    	    {
                    	     
                    	    	        newCompositeNode(grammarAccess.getWMLPreprocIFAccess().getElseExpressionsWMLValuedExpressionParserRuleCall_2_1_0()); 
                    	    	    
                    	    pushFollow(FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1571);
                    	    lv_ElseExpressions_3_0=ruleWMLValuedExpression();

                    	    state._fsp--;


                    	    	        if (current==null) {
                    	    	            current = createModelElementForParent(grammarAccess.getWMLPreprocIFRule());
                    	    	        }
                    	           		add(
                    	           			current, 
                    	           			"ElseExpressions",
                    	            		lv_ElseExpressions_3_0, 
                    	            		"WMLValuedExpression");
                    	    	        afterParserOrEnumRuleCall();
                    	    	    

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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:799:5: ( (lv_endName_4_0= RULE_ENDIF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:800:1: (lv_endName_4_0= RULE_ENDIF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:800:1: (lv_endName_4_0= RULE_ENDIF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:801:3: lv_endName_4_0= RULE_ENDIF
            {
            lv_endName_4_0=(Token)match(input,RULE_ENDIF,FOLLOW_RULE_ENDIF_in_ruleWMLPreprocIF1591); 

            			newLeafNode(lv_endName_4_0, grammarAccess.getWMLPreprocIFAccess().getEndNameENDIFTerminalRuleCall_3_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLPreprocIFRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"endName",
                    		lv_endName_4_0, 
                    		"ENDIF");
            	    

            }


            }


            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLPreprocIF"


    // $ANTLR start "entryRuleWMLRootExpression"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:825:1: entryRuleWMLRootExpression returns [EObject current=null] : iv_ruleWMLRootExpression= ruleWMLRootExpression EOF ;
    public final EObject entryRuleWMLRootExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLRootExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:826:2: (iv_ruleWMLRootExpression= ruleWMLRootExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:827:2: iv_ruleWMLRootExpression= ruleWMLRootExpression EOF
            {
             newCompositeNode(grammarAccess.getWMLRootExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression1632);
            iv_ruleWMLRootExpression=ruleWMLRootExpression();

            state._fsp--;

             current =iv_ruleWMLRootExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLRootExpression1642); 

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
    // $ANTLR end "entryRuleWMLRootExpression"


    // $ANTLR start "ruleWMLRootExpression"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:834:1: ruleWMLRootExpression returns [EObject current=null] : (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF ) ;
    public final EObject ruleWMLRootExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLTag_0 = null;

        EObject this_WMLMacroCall_1 = null;

        EObject this_WMLMacroDefine_2 = null;

        EObject this_WMLTextdomain_3 = null;

        EObject this_WMLPreprocIF_4 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:837:28: ( (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:838:1: (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:838:1: (this_WMLTag_0= ruleWMLTag | this_WMLMacroCall_1= ruleWMLMacroCall | this_WMLMacroDefine_2= ruleWMLMacroDefine | this_WMLTextdomain_3= ruleWMLTextdomain | this_WMLPreprocIF_4= ruleWMLPreprocIF )
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
                    new NoViableAltException("", 21, 0, input);

                throw nvae;
            }

            switch (alt21) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:839:5: this_WMLTag_0= ruleWMLTag
                    {
                     
                            newCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLTagParserRuleCall_0()); 
                        
                    pushFollow(FOLLOW_ruleWMLTag_in_ruleWMLRootExpression1689);
                    this_WMLTag_0=ruleWMLTag();

                    state._fsp--;

                     
                            current = this_WMLTag_0; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:849:5: this_WMLMacroCall_1= ruleWMLMacroCall
                    {
                     
                            newCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLMacroCallParserRuleCall_1()); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroCall_in_ruleWMLRootExpression1716);
                    this_WMLMacroCall_1=ruleWMLMacroCall();

                    state._fsp--;

                     
                            current = this_WMLMacroCall_1; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:859:5: this_WMLMacroDefine_2= ruleWMLMacroDefine
                    {
                     
                            newCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLMacroDefineParserRuleCall_2()); 
                        
                    pushFollow(FOLLOW_ruleWMLMacroDefine_in_ruleWMLRootExpression1743);
                    this_WMLMacroDefine_2=ruleWMLMacroDefine();

                    state._fsp--;

                     
                            current = this_WMLMacroDefine_2; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:869:5: this_WMLTextdomain_3= ruleWMLTextdomain
                    {
                     
                            newCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLTextdomainParserRuleCall_3()); 
                        
                    pushFollow(FOLLOW_ruleWMLTextdomain_in_ruleWMLRootExpression1770);
                    this_WMLTextdomain_3=ruleWMLTextdomain();

                    state._fsp--;

                     
                            current = this_WMLTextdomain_3; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:879:5: this_WMLPreprocIF_4= ruleWMLPreprocIF
                    {
                     
                            newCompositeNode(grammarAccess.getWMLRootExpressionAccess().getWMLPreprocIFParserRuleCall_4()); 
                        
                    pushFollow(FOLLOW_ruleWMLPreprocIF_in_ruleWMLRootExpression1797);
                    this_WMLPreprocIF_4=ruleWMLPreprocIF();

                    state._fsp--;

                     
                            current = this_WMLPreprocIF_4; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLRootExpression"


    // $ANTLR start "entryRuleWMLExpression"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:895:1: entryRuleWMLExpression returns [EObject current=null] : iv_ruleWMLExpression= ruleWMLExpression EOF ;
    public final EObject entryRuleWMLExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:896:2: (iv_ruleWMLExpression= ruleWMLExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:897:2: iv_ruleWMLExpression= ruleWMLExpression EOF
            {
             newCompositeNode(grammarAccess.getWMLExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression1832);
            iv_ruleWMLExpression=ruleWMLExpression();

            state._fsp--;

             current =iv_ruleWMLExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLExpression1842); 

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
    // $ANTLR end "entryRuleWMLExpression"


    // $ANTLR start "ruleWMLExpression"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:904:1: ruleWMLExpression returns [EObject current=null] : (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey ) ;
    public final EObject ruleWMLExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLRootExpression_0 = null;

        EObject this_WMLKey_1 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:907:28: ( (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:908:1: (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:908:1: (this_WMLRootExpression_0= ruleWMLRootExpression | this_WMLKey_1= ruleWMLKey )
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
                    new NoViableAltException("", 22, 0, input);

                throw nvae;
            }
            switch (alt22) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:909:5: this_WMLRootExpression_0= ruleWMLRootExpression
                    {
                     
                            newCompositeNode(grammarAccess.getWMLExpressionAccess().getWMLRootExpressionParserRuleCall_0()); 
                        
                    pushFollow(FOLLOW_ruleWMLRootExpression_in_ruleWMLExpression1889);
                    this_WMLRootExpression_0=ruleWMLRootExpression();

                    state._fsp--;

                     
                            current = this_WMLRootExpression_0; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:919:5: this_WMLKey_1= ruleWMLKey
                    {
                     
                            newCompositeNode(grammarAccess.getWMLExpressionAccess().getWMLKeyParserRuleCall_1()); 
                        
                    pushFollow(FOLLOW_ruleWMLKey_in_ruleWMLExpression1916);
                    this_WMLKey_1=ruleWMLKey();

                    state._fsp--;

                     
                            current = this_WMLKey_1; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLExpression"


    // $ANTLR start "entryRuleWMLValuedExpression"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:935:1: entryRuleWMLValuedExpression returns [EObject current=null] : iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF ;
    public final EObject entryRuleWMLValuedExpression() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLValuedExpression = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:936:2: (iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:937:2: iv_ruleWMLValuedExpression= ruleWMLValuedExpression EOF
            {
             newCompositeNode(grammarAccess.getWMLValuedExpressionRule()); 
            pushFollow(FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression1951);
            iv_ruleWMLValuedExpression=ruleWMLValuedExpression();

            state._fsp--;

             current =iv_ruleWMLValuedExpression; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValuedExpression1961); 

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
    // $ANTLR end "entryRuleWMLValuedExpression"


    // $ANTLR start "ruleWMLValuedExpression"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:944:1: ruleWMLValuedExpression returns [EObject current=null] : (this_WMLExpression_0= ruleWMLExpression | this_WMLValue_1= ruleWMLValue ) ;
    public final EObject ruleWMLValuedExpression() throws RecognitionException {
        EObject current = null;

        EObject this_WMLExpression_0 = null;

        EObject this_WMLValue_1 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:947:28: ( (this_WMLExpression_0= ruleWMLExpression | this_WMLValue_1= ruleWMLValue ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:948:1: (this_WMLExpression_0= ruleWMLExpression | this_WMLValue_1= ruleWMLValue )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:948:1: (this_WMLExpression_0= ruleWMLExpression | this_WMLValue_1= ruleWMLValue )
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
                        new NoViableAltException("", 23, 2, input);

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
                    new NoViableAltException("", 23, 0, input);

                throw nvae;
            }

            switch (alt23) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:949:5: this_WMLExpression_0= ruleWMLExpression
                    {
                     
                            newCompositeNode(grammarAccess.getWMLValuedExpressionAccess().getWMLExpressionParserRuleCall_0()); 
                        
                    pushFollow(FOLLOW_ruleWMLExpression_in_ruleWMLValuedExpression2008);
                    this_WMLExpression_0=ruleWMLExpression();

                    state._fsp--;

                     
                            current = this_WMLExpression_0; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:959:5: this_WMLValue_1= ruleWMLValue
                    {
                     
                            newCompositeNode(grammarAccess.getWMLValuedExpressionAccess().getWMLValueParserRuleCall_1()); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLValuedExpression2035);
                    this_WMLValue_1=ruleWMLValue();

                    state._fsp--;

                     
                            current = this_WMLValue_1; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLValuedExpression"


    // $ANTLR start "entryRuleWMLTextdomain"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:975:1: entryRuleWMLTextdomain returns [EObject current=null] : iv_ruleWMLTextdomain= ruleWMLTextdomain EOF ;
    public final EObject entryRuleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLTextdomain = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:976:2: (iv_ruleWMLTextdomain= ruleWMLTextdomain EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:977:2: iv_ruleWMLTextdomain= ruleWMLTextdomain EOF
            {
             newCompositeNode(grammarAccess.getWMLTextdomainRule()); 
            pushFollow(FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain2070);
            iv_ruleWMLTextdomain=ruleWMLTextdomain();

            state._fsp--;

             current =iv_ruleWMLTextdomain; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLTextdomain2080); 

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
    // $ANTLR end "entryRuleWMLTextdomain"


    // $ANTLR start "ruleWMLTextdomain"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:984:1: ruleWMLTextdomain returns [EObject current=null] : ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) ;
    public final EObject ruleWMLTextdomain() throws RecognitionException {
        EObject current = null;

        Token lv_name_0_0=null;

         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:987:28: ( ( (lv_name_0_0= RULE_TEXTDOMAIN ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:988:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:988:1: ( (lv_name_0_0= RULE_TEXTDOMAIN ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:989:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:989:1: (lv_name_0_0= RULE_TEXTDOMAIN )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:990:3: lv_name_0_0= RULE_TEXTDOMAIN
            {
            lv_name_0_0=(Token)match(input,RULE_TEXTDOMAIN,FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain2121); 

            			newLeafNode(lv_name_0_0, grammarAccess.getWMLTextdomainAccess().getNameTEXTDOMAINTerminalRuleCall_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLTextdomainRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"name",
                    		lv_name_0_0, 
                    		"TEXTDOMAIN");
            	    

            }


            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLTextdomain"


    // $ANTLR start "entryRuleWMLLuaCode"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1014:1: entryRuleWMLLuaCode returns [EObject current=null] : iv_ruleWMLLuaCode= ruleWMLLuaCode EOF ;
    public final EObject entryRuleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLLuaCode = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1015:2: (iv_ruleWMLLuaCode= ruleWMLLuaCode EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1016:2: iv_ruleWMLLuaCode= ruleWMLLuaCode EOF
            {
             newCompositeNode(grammarAccess.getWMLLuaCodeRule()); 
            pushFollow(FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode2161);
            iv_ruleWMLLuaCode=ruleWMLLuaCode();

            state._fsp--;

             current =iv_ruleWMLLuaCode; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLLuaCode2171); 

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
    // $ANTLR end "entryRuleWMLLuaCode"


    // $ANTLR start "ruleWMLLuaCode"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1023:1: ruleWMLLuaCode returns [EObject current=null] : ( (lv_value_0_0= RULE_LUA_CODE ) ) ;
    public final EObject ruleWMLLuaCode() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_0=null;

         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1026:28: ( ( (lv_value_0_0= RULE_LUA_CODE ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1027:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1027:1: ( (lv_value_0_0= RULE_LUA_CODE ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1028:1: (lv_value_0_0= RULE_LUA_CODE )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1028:1: (lv_value_0_0= RULE_LUA_CODE )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1029:3: lv_value_0_0= RULE_LUA_CODE
            {
            lv_value_0_0=(Token)match(input,RULE_LUA_CODE,FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode2212); 

            			newLeafNode(lv_value_0_0, grammarAccess.getWMLLuaCodeAccess().getValueLUA_CODETerminalRuleCall_0()); 
            		

            	        if (current==null) {
            	            current = createModelElement(grammarAccess.getWMLLuaCodeRule());
            	        }
                   		setWithLastConsumed(
                   			current, 
                   			"value",
                    		lv_value_0_0, 
                    		"LUA_CODE");
            	    

            }


            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLLuaCode"


    // $ANTLR start "entryRuleWMLMacroParameter"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1053:1: entryRuleWMLMacroParameter returns [EObject current=null] : iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF ;
    public final EObject entryRuleWMLMacroParameter() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLMacroParameter = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1054:2: (iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1055:2: iv_ruleWMLMacroParameter= ruleWMLMacroParameter EOF
            {
             newCompositeNode(grammarAccess.getWMLMacroParameterRule()); 
            pushFollow(FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter2252);
            iv_ruleWMLMacroParameter=ruleWMLMacroParameter();

            state._fsp--;

             current =iv_ruleWMLMacroParameter; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLMacroParameter2262); 

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
    // $ANTLR end "entryRuleWMLMacroParameter"


    // $ANTLR start "ruleWMLMacroParameter"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1062:1: ruleWMLMacroParameter returns [EObject current=null] : (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens ) ;
    public final EObject ruleWMLMacroParameter() throws RecognitionException {
        EObject current = null;

        EObject this_WMLValue_0 = null;

        EObject this_MacroTokens_1 = null;


         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1065:28: ( (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1066:1: (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1066:1: (this_WMLValue_0= ruleWMLValue | this_MacroTokens_1= ruleMacroTokens )
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
                    new NoViableAltException("", 24, 0, input);

                throw nvae;
            }
            switch (alt24) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1067:5: this_WMLValue_0= ruleWMLValue
                    {
                     
                            newCompositeNode(grammarAccess.getWMLMacroParameterAccess().getWMLValueParserRuleCall_0()); 
                        
                    pushFollow(FOLLOW_ruleWMLValue_in_ruleWMLMacroParameter2309);
                    this_WMLValue_0=ruleWMLValue();

                    state._fsp--;

                     
                            current = this_WMLValue_0; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1077:5: this_MacroTokens_1= ruleMacroTokens
                    {
                     
                            newCompositeNode(grammarAccess.getWMLMacroParameterAccess().getMacroTokensParserRuleCall_1()); 
                        
                    pushFollow(FOLLOW_ruleMacroTokens_in_ruleWMLMacroParameter2336);
                    this_MacroTokens_1=ruleMacroTokens();

                    state._fsp--;

                     
                            current = this_MacroTokens_1; 
                            afterParserOrEnumRuleCall();
                        

                    }
                    break;

            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLMacroParameter"


    // $ANTLR start "entryRuleWMLValue"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1093:1: entryRuleWMLValue returns [EObject current=null] : iv_ruleWMLValue= ruleWMLValue EOF ;
    public final EObject entryRuleWMLValue() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleWMLValue = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1094:2: (iv_ruleWMLValue= ruleWMLValue EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1095:2: iv_ruleWMLValue= ruleWMLValue EOF
            {
             newCompositeNode(grammarAccess.getWMLValueRule()); 
            pushFollow(FOLLOW_ruleWMLValue_in_entryRuleWMLValue2371);
            iv_ruleWMLValue=ruleWMLValue();

            state._fsp--;

             current =iv_ruleWMLValue; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleWMLValue2381); 

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
    // $ANTLR end "entryRuleWMLValue"


    // $ANTLR start "ruleWMLValue"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1102:1: ruleWMLValue returns [EObject current=null] : ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '_' | lv_value_0_4= '~' | lv_value_0_5= '.' | lv_value_0_6= './' | lv_value_0_7= '$' | lv_value_0_8= '/' | lv_value_0_9= '(' | lv_value_0_10= ')' | lv_value_0_11= RULE_ANY_OTHER ) ) ) ;
    public final EObject ruleWMLValue() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_1=null;
        Token lv_value_0_2=null;
        Token lv_value_0_3=null;
        Token lv_value_0_4=null;
        Token lv_value_0_5=null;
        Token lv_value_0_6=null;
        Token lv_value_0_7=null;
        Token lv_value_0_8=null;
        Token lv_value_0_9=null;
        Token lv_value_0_10=null;
        Token lv_value_0_11=null;

         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1105:28: ( ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '_' | lv_value_0_4= '~' | lv_value_0_5= '.' | lv_value_0_6= './' | lv_value_0_7= '$' | lv_value_0_8= '/' | lv_value_0_9= '(' | lv_value_0_10= ')' | lv_value_0_11= RULE_ANY_OTHER ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1106:1: ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '_' | lv_value_0_4= '~' | lv_value_0_5= '.' | lv_value_0_6= './' | lv_value_0_7= '$' | lv_value_0_8= '/' | lv_value_0_9= '(' | lv_value_0_10= ')' | lv_value_0_11= RULE_ANY_OTHER ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1106:1: ( ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '_' | lv_value_0_4= '~' | lv_value_0_5= '.' | lv_value_0_6= './' | lv_value_0_7= '$' | lv_value_0_8= '/' | lv_value_0_9= '(' | lv_value_0_10= ')' | lv_value_0_11= RULE_ANY_OTHER ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1107:1: ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '_' | lv_value_0_4= '~' | lv_value_0_5= '.' | lv_value_0_6= './' | lv_value_0_7= '$' | lv_value_0_8= '/' | lv_value_0_9= '(' | lv_value_0_10= ')' | lv_value_0_11= RULE_ANY_OTHER ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1107:1: ( (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '_' | lv_value_0_4= '~' | lv_value_0_5= '.' | lv_value_0_6= './' | lv_value_0_7= '$' | lv_value_0_8= '/' | lv_value_0_9= '(' | lv_value_0_10= ')' | lv_value_0_11= RULE_ANY_OTHER ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1108:1: (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '_' | lv_value_0_4= '~' | lv_value_0_5= '.' | lv_value_0_6= './' | lv_value_0_7= '$' | lv_value_0_8= '/' | lv_value_0_9= '(' | lv_value_0_10= ')' | lv_value_0_11= RULE_ANY_OTHER )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1108:1: (lv_value_0_1= RULE_ID | lv_value_0_2= RULE_STRING | lv_value_0_3= '_' | lv_value_0_4= '~' | lv_value_0_5= '.' | lv_value_0_6= './' | lv_value_0_7= '$' | lv_value_0_8= '/' | lv_value_0_9= '(' | lv_value_0_10= ')' | lv_value_0_11= RULE_ANY_OTHER )
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
                    new NoViableAltException("", 25, 0, input);

                throw nvae;
            }

            switch (alt25) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1109:3: lv_value_0_1= RULE_ID
                    {
                    lv_value_0_1=(Token)match(input,RULE_ID,FOLLOW_RULE_ID_in_ruleWMLValue2424); 

                    			newLeafNode(lv_value_0_1, grammarAccess.getWMLValueAccess().getValueIDTerminalRuleCall_0_0()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"value",
                            		lv_value_0_1, 
                            		"ID");
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1124:8: lv_value_0_2= RULE_STRING
                    {
                    lv_value_0_2=(Token)match(input,RULE_STRING,FOLLOW_RULE_STRING_in_ruleWMLValue2444); 

                    			newLeafNode(lv_value_0_2, grammarAccess.getWMLValueAccess().getValueSTRINGTerminalRuleCall_0_1()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"value",
                            		lv_value_0_2, 
                            		"STRING");
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1139:8: lv_value_0_3= '_'
                    {
                    lv_value_0_3=(Token)match(input,29,FOLLOW_29_in_ruleWMLValue2465); 

                            newLeafNode(lv_value_0_3, grammarAccess.getWMLValueAccess().getValue_Keyword_0_2());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_3, null);
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1151:8: lv_value_0_4= '~'
                    {
                    lv_value_0_4=(Token)match(input,27,FOLLOW_27_in_ruleWMLValue2494); 

                            newLeafNode(lv_value_0_4, grammarAccess.getWMLValueAccess().getValueTildeKeyword_0_3());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_4, null);
                    	    

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1163:8: lv_value_0_5= '.'
                    {
                    lv_value_0_5=(Token)match(input,30,FOLLOW_30_in_ruleWMLValue2523); 

                            newLeafNode(lv_value_0_5, grammarAccess.getWMLValueAccess().getValueFullStopKeyword_0_4());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_5, null);
                    	    

                    }
                    break;
                case 6 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1175:8: lv_value_0_6= './'
                    {
                    lv_value_0_6=(Token)match(input,26,FOLLOW_26_in_ruleWMLValue2552); 

                            newLeafNode(lv_value_0_6, grammarAccess.getWMLValueAccess().getValueFullStopSolidusKeyword_0_5());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_6, null);
                    	    

                    }
                    break;
                case 7 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1187:8: lv_value_0_7= '$'
                    {
                    lv_value_0_7=(Token)match(input,31,FOLLOW_31_in_ruleWMLValue2581); 

                            newLeafNode(lv_value_0_7, grammarAccess.getWMLValueAccess().getValueDollarSignKeyword_0_6());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_7, null);
                    	    

                    }
                    break;
                case 8 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1199:8: lv_value_0_8= '/'
                    {
                    lv_value_0_8=(Token)match(input,32,FOLLOW_32_in_ruleWMLValue2610); 

                            newLeafNode(lv_value_0_8, grammarAccess.getWMLValueAccess().getValueSolidusKeyword_0_7());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_8, null);
                    	    

                    }
                    break;
                case 9 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1211:8: lv_value_0_9= '('
                    {
                    lv_value_0_9=(Token)match(input,33,FOLLOW_33_in_ruleWMLValue2639); 

                            newLeafNode(lv_value_0_9, grammarAccess.getWMLValueAccess().getValueLeftParenthesisKeyword_0_8());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_9, null);
                    	    

                    }
                    break;
                case 10 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1223:8: lv_value_0_10= ')'
                    {
                    lv_value_0_10=(Token)match(input,34,FOLLOW_34_in_ruleWMLValue2668); 

                            newLeafNode(lv_value_0_10, grammarAccess.getWMLValueAccess().getValueRightParenthesisKeyword_0_9());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_10, null);
                    	    

                    }
                    break;
                case 11 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1235:8: lv_value_0_11= RULE_ANY_OTHER
                    {
                    lv_value_0_11=(Token)match(input,RULE_ANY_OTHER,FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue2696); 

                    			newLeafNode(lv_value_0_11, grammarAccess.getWMLValueAccess().getValueANY_OTHERTerminalRuleCall_0_10()); 
                    		

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getWMLValueRule());
                    	        }
                           		setWithLastConsumed(
                           			current, 
                           			"value",
                            		lv_value_0_11, 
                            		"ANY_OTHER");
                    	    

                    }
                    break;

            }


            }


            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleWMLValue"


    // $ANTLR start "entryRuleMacroTokens"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1261:1: entryRuleMacroTokens returns [EObject current=null] : iv_ruleMacroTokens= ruleMacroTokens EOF ;
    public final EObject entryRuleMacroTokens() throws RecognitionException {
        EObject current = null;

        EObject iv_ruleMacroTokens = null;


        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1262:2: (iv_ruleMacroTokens= ruleMacroTokens EOF )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1263:2: iv_ruleMacroTokens= ruleMacroTokens EOF
            {
             newCompositeNode(grammarAccess.getMacroTokensRule()); 
            pushFollow(FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens2739);
            iv_ruleMacroTokens=ruleMacroTokens();

            state._fsp--;

             current =iv_ruleMacroTokens; 
            match(input,EOF,FOLLOW_EOF_in_entryRuleMacroTokens2749); 

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
    // $ANTLR end "entryRuleMacroTokens"


    // $ANTLR start "ruleMacroTokens"
    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1270:1: ruleMacroTokens returns [EObject current=null] : ( ( (lv_value_0_1= '=' | lv_value_0_2= '[' | lv_value_0_3= ']' | lv_value_0_4= '+' | lv_value_0_5= '[/' ) ) ) ;
    public final EObject ruleMacroTokens() throws RecognitionException {
        EObject current = null;

        Token lv_value_0_1=null;
        Token lv_value_0_2=null;
        Token lv_value_0_3=null;
        Token lv_value_0_4=null;
        Token lv_value_0_5=null;

         enterRule(); 
            
        try {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1273:28: ( ( ( (lv_value_0_1= '=' | lv_value_0_2= '[' | lv_value_0_3= ']' | lv_value_0_4= '+' | lv_value_0_5= '[/' ) ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1274:1: ( ( (lv_value_0_1= '=' | lv_value_0_2= '[' | lv_value_0_3= ']' | lv_value_0_4= '+' | lv_value_0_5= '[/' ) ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1274:1: ( ( (lv_value_0_1= '=' | lv_value_0_2= '[' | lv_value_0_3= ']' | lv_value_0_4= '+' | lv_value_0_5= '[/' ) ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1275:1: ( (lv_value_0_1= '=' | lv_value_0_2= '[' | lv_value_0_3= ']' | lv_value_0_4= '+' | lv_value_0_5= '[/' ) )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1275:1: ( (lv_value_0_1= '=' | lv_value_0_2= '[' | lv_value_0_3= ']' | lv_value_0_4= '+' | lv_value_0_5= '[/' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1276:1: (lv_value_0_1= '=' | lv_value_0_2= '[' | lv_value_0_3= ']' | lv_value_0_4= '+' | lv_value_0_5= '[/' )
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1276:1: (lv_value_0_1= '=' | lv_value_0_2= '[' | lv_value_0_3= ']' | lv_value_0_4= '+' | lv_value_0_5= '[/' )
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
                    new NoViableAltException("", 26, 0, input);

                throw nvae;
            }

            switch (alt26) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1277:3: lv_value_0_1= '='
                    {
                    lv_value_0_1=(Token)match(input,24,FOLLOW_24_in_ruleMacroTokens2793); 

                            newLeafNode(lv_value_0_1, grammarAccess.getMacroTokensAccess().getValueEqualsSignKeyword_0_0());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getMacroTokensRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_1, null);
                    	    

                    }
                    break;
                case 2 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1289:8: lv_value_0_2= '['
                    {
                    lv_value_0_2=(Token)match(input,20,FOLLOW_20_in_ruleMacroTokens2822); 

                            newLeafNode(lv_value_0_2, grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketKeyword_0_1());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getMacroTokensRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_2, null);
                    	    

                    }
                    break;
                case 3 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1301:8: lv_value_0_3= ']'
                    {
                    lv_value_0_3=(Token)match(input,22,FOLLOW_22_in_ruleMacroTokens2851); 

                            newLeafNode(lv_value_0_3, grammarAccess.getMacroTokensAccess().getValueRightSquareBracketKeyword_0_2());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getMacroTokensRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_3, null);
                    	    

                    }
                    break;
                case 4 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1313:8: lv_value_0_4= '+'
                    {
                    lv_value_0_4=(Token)match(input,21,FOLLOW_21_in_ruleMacroTokens2880); 

                            newLeafNode(lv_value_0_4, grammarAccess.getMacroTokensAccess().getValuePlusSignKeyword_0_3());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getMacroTokensRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_4, null);
                    	    

                    }
                    break;
                case 5 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1325:8: lv_value_0_5= '[/'
                    {
                    lv_value_0_5=(Token)match(input,23,FOLLOW_23_in_ruleMacroTokens2909); 

                            newLeafNode(lv_value_0_5, grammarAccess.getMacroTokensAccess().getValueLeftSquareBracketSolidusKeyword_0_4());
                        

                    	        if (current==null) {
                    	            current = createModelElement(grammarAccess.getMacroTokensRule());
                    	        }
                           		setWithLastConsumed(current, "value", lv_value_0_5, null);
                    	    

                    }
                    break;

            }


            }


            }


            }

             leaveRule(); 
        }
         
            catch (RecognitionException re) { 
                recover(input,re); 
                appendSkippedTokens();
            } 
        finally {
        }
        return current;
    }
    // $ANTLR end "ruleMacroTokens"

    // Delegated rules


 

    public static final BitSet FOLLOW_ruleWMLRoot_in_entryRuleWMLRoot75 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRoot85 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_ruleWMLRoot130 = new BitSet(new long[]{0x0000000002109E82L});
    public static final BitSet FOLLOW_ruleWMLTag_in_entryRuleWMLTag166 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTag176 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleWMLTag213 = new BitSet(new long[]{0x0000000000200010L});
    public static final BitSet FOLLOW_21_in_ruleWMLTag231 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag262 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_22_in_ruleWMLTag279 = new BitSet(new long[]{0x0000000002909E90L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_ruleWMLTag300 = new BitSet(new long[]{0x0000000002909E90L});
    public static final BitSet FOLLOW_23_in_ruleWMLTag313 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLTag330 = new BitSet(new long[]{0x0000000000400000L});
    public static final BitSet FOLLOW_22_in_ruleWMLTag347 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_entryRuleWMLKey389 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKey399 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLKey445 = new BitSet(new long[]{0x0000000001000000L});
    public static final BitSet FOLLOW_24_in_ruleWMLKey462 = new BitSet(new long[]{0x00000007EE370070L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey483 = new BitSet(new long[]{0x00000007EE370070L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey497 = new BitSet(new long[]{0x0000000000200000L});
    public static final BitSet FOLLOW_21_in_ruleWMLKey510 = new BitSet(new long[]{0x00000007EE170030L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey522 = new BitSet(new long[]{0x00000007EE170010L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_ruleWMLKey544 = new BitSet(new long[]{0x00000007EE370070L});
    public static final BitSet FOLLOW_RULE_EOL_in_ruleWMLKey566 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_SL_COMMENT_in_ruleWMLKey586 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKeyValue_in_entryRuleWMLKeyValue634 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLKeyValue644 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLKeyValue691 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLKeyValue718 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_ruleWMLKeyValue745 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_ruleWMLKeyValue772 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_entryRuleWMLMacroCall807 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCall817 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_25_in_ruleWMLMacroCall854 = new BitSet(new long[]{0x000000000C000010L});
    public static final BitSet FOLLOW_26_in_ruleWMLMacroCall872 = new BitSet(new long[]{0x0000000008000010L});
    public static final BitSet FOLLOW_27_in_ruleWMLMacroCall904 = new BitSet(new long[]{0x0000000000000010L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLMacroCall935 = new BitSet(new long[]{0x00000007FFF60010L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_ruleWMLMacroCall961 = new BitSet(new long[]{0x00000007FFF60010L});
    public static final BitSet FOLLOW_28_in_ruleWMLMacroCall974 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCallParameter_in_entryRuleWMLMacroCallParameter1010 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroCallParameter1020 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_ruleWMLMacroCallParameter1067 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLMacroCallParameter1094 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLArrayCall_in_entryRuleWMLArrayCall1129 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLArrayCall1139 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleWMLArrayCall1176 = new BitSet(new long[]{0x00000007EC060010L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLArrayCall1197 = new BitSet(new long[]{0x00000007EC460010L});
    public static final BitSet FOLLOW_22_in_ruleWMLArrayCall1210 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_entryRuleWMLMacroDefine1246 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroDefine1256 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_DEFINE_in_ruleWMLMacroDefine1298 = new BitSet(new long[]{0x00000007EE169F90L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_ruleWMLMacroDefine1324 = new BitSet(new long[]{0x00000007EE169F90L});
    public static final BitSet FOLLOW_RULE_ENDDEF_in_ruleWMLMacroDefine1342 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_entryRuleWMLPreprocIF1383 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLPreprocIF1393 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_IFDEF_in_ruleWMLPreprocIF1437 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFNDEF_in_ruleWMLPreprocIF1457 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFHAVE_in_ruleWMLPreprocIF1477 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_IFNHAVE_in_ruleWMLPreprocIF1497 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1526 = new BitSet(new long[]{0x00000007EE16FE90L});
    public static final BitSet FOLLOW_RULE_ELSE_in_ruleWMLPreprocIF1545 = new BitSet(new long[]{0x00000007EE169E90L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_ruleWMLPreprocIF1571 = new BitSet(new long[]{0x00000007EE16DE90L});
    public static final BitSet FOLLOW_RULE_ENDIF_in_ruleWMLPreprocIF1591 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_entryRuleWMLRootExpression1632 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLRootExpression1642 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTag_in_ruleWMLRootExpression1689 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroCall_in_ruleWMLRootExpression1716 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroDefine_in_ruleWMLRootExpression1743 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_ruleWMLRootExpression1770 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLPreprocIF_in_ruleWMLRootExpression1797 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_entryRuleWMLExpression1832 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLExpression1842 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLRootExpression_in_ruleWMLExpression1889 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLKey_in_ruleWMLExpression1916 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValuedExpression_in_entryRuleWMLValuedExpression1951 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValuedExpression1961 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLExpression_in_ruleWMLValuedExpression2008 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLValuedExpression2035 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLTextdomain_in_entryRuleWMLTextdomain2070 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLTextdomain2080 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_TEXTDOMAIN_in_ruleWMLTextdomain2121 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLLuaCode_in_entryRuleWMLLuaCode2161 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLLuaCode2171 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_LUA_CODE_in_ruleWMLLuaCode2212 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLMacroParameter_in_entryRuleWMLMacroParameter2252 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLMacroParameter2262 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_ruleWMLMacroParameter2309 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_ruleWMLMacroParameter2336 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleWMLValue_in_entryRuleWMLValue2371 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleWMLValue2381 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ID_in_ruleWMLValue2424 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_STRING_in_ruleWMLValue2444 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_29_in_ruleWMLValue2465 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_27_in_ruleWMLValue2494 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_30_in_ruleWMLValue2523 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_26_in_ruleWMLValue2552 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_31_in_ruleWMLValue2581 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_32_in_ruleWMLValue2610 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_33_in_ruleWMLValue2639 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_34_in_ruleWMLValue2668 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_RULE_ANY_OTHER_in_ruleWMLValue2696 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_ruleMacroTokens_in_entryRuleMacroTokens2739 = new BitSet(new long[]{0x0000000000000000L});
    public static final BitSet FOLLOW_EOF_in_entryRuleMacroTokens2749 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_24_in_ruleMacroTokens2793 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_20_in_ruleMacroTokens2822 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_22_in_ruleMacroTokens2851 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_21_in_ruleMacroTokens2880 = new BitSet(new long[]{0x0000000000000002L});
    public static final BitSet FOLLOW_23_in_ruleMacroTokens2909 = new BitSet(new long[]{0x0000000000000002L});

}