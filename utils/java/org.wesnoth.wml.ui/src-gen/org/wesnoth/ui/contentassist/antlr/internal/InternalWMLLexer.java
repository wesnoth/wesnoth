package org.wesnoth.ui.contentassist.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.ui.editor.contentassist.antlr.internal.Lexer;


import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

@SuppressWarnings("all")
public class InternalWMLLexer extends Lexer {
    public static final int RULE_LUA_CODE=9;
    public static final int RULE_ID=6;
    public static final int RULE_ANY_OTHER=8;
    public static final int RULE_EOL=4;
    public static final int RULE_TEXTDOMAIN=11;
    public static final int Tokens=21;
    public static final int EOF=-1;
    public static final int RULE_SL_COMMENT=5;
    public static final int T20=20;
    public static final int RULE_STRING=7;
    public static final int RULE_DEFINE=10;
    public static final int T13=13;
    public static final int T14=14;
    public static final int RULE_WS=12;
    public static final int T15=15;
    public static final int T16=16;
    public static final int T17=17;
    public static final int T18=18;
    public static final int T19=19;
    public InternalWMLLexer() {;} 
    public InternalWMLLexer(CharStream input) {
        super(input);
    }
    public String getGrammarFileName() { return "../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g"; }

    // $ANTLR start T13
    public final void mT13() throws RecognitionException {
        try {
            int _type = T13;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:10:5: ( '+' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:10:7: '+'
            {
            match('+'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T13

    // $ANTLR start T14
    public final void mT14() throws RecognitionException {
        try {
            int _type = T14;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:11:5: ( '[' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:11:7: '['
            {
            match('['); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T14

    // $ANTLR start T15
    public final void mT15() throws RecognitionException {
        try {
            int _type = T15;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:12:5: ( ']' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:12:7: ']'
            {
            match(']'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T15

    // $ANTLR start T16
    public final void mT16() throws RecognitionException {
        try {
            int _type = T16;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:13:5: ( '[/' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:13:7: '[/'
            {
            match("[/"); 


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T16

    // $ANTLR start T17
    public final void mT17() throws RecognitionException {
        try {
            int _type = T17;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:14:5: ( '=' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:14:7: '='
            {
            match('='); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T17

    // $ANTLR start T18
    public final void mT18() throws RecognitionException {
        try {
            int _type = T18;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:15:5: ( '{' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:15:7: '{'
            {
            match('{'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T18

    // $ANTLR start T19
    public final void mT19() throws RecognitionException {
        try {
            int _type = T19;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:16:5: ( '}' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:16:7: '}'
            {
            match('}'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T19

    // $ANTLR start T20
    public final void mT20() throws RecognitionException {
        try {
            int _type = T20;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:17:5: ( '~' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:17:7: '~'
            {
            match('~'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T20

    // $ANTLR start RULE_LUA_CODE
    public final void mRULE_LUA_CODE() throws RecognitionException {
        try {
            int _type = RULE_LUA_CODE;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:15: ( '<<' ( options {greedy=false; } : . )* '>>' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:17: '<<' ( options {greedy=false; } : . )* '>>'
            {
            match("<<"); 

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:22: ( options {greedy=false; } : . )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0=='>') ) {
                    int LA1_1 = input.LA(2);

                    if ( (LA1_1=='>') ) {
                        alt1=2;
                    }
                    else if ( ((LA1_1>='\u0000' && LA1_1<='=')||(LA1_1>='?' && LA1_1<='\uFFFE')) ) {
                        alt1=1;
                    }


                }
                else if ( ((LA1_0>='\u0000' && LA1_0<='=')||(LA1_0>='?' && LA1_0<='\uFFFE')) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1556:50: .
            	    {
            	    matchAny(); 

            	    }
            	    break;

            	default :
            	    break loop1;
                }
            } while (true);

            match(">>"); 


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_LUA_CODE

    // $ANTLR start RULE_DEFINE
    public final void mRULE_DEFINE() throws RecognitionException {
        try {
            int _type = RULE_DEFINE;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:13: ( '#define' ( options {greedy=false; } : . )* '#enddef' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:15: '#define' ( options {greedy=false; } : . )* '#enddef'
            {
            match("#define"); 

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:25: ( options {greedy=false; } : . )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( (LA2_0=='#') ) {
                    int LA2_1 = input.LA(2);

                    if ( (LA2_1=='e') ) {
                        int LA2_3 = input.LA(3);

                        if ( (LA2_3=='n') ) {
                            int LA2_4 = input.LA(4);

                            if ( (LA2_4=='d') ) {
                                int LA2_5 = input.LA(5);

                                if ( (LA2_5=='d') ) {
                                    int LA2_6 = input.LA(6);

                                    if ( (LA2_6=='e') ) {
                                        int LA2_7 = input.LA(7);

                                        if ( (LA2_7=='f') ) {
                                            alt2=2;
                                        }
                                        else if ( ((LA2_7>='\u0000' && LA2_7<='e')||(LA2_7>='g' && LA2_7<='\uFFFE')) ) {
                                            alt2=1;
                                        }


                                    }
                                    else if ( ((LA2_6>='\u0000' && LA2_6<='d')||(LA2_6>='f' && LA2_6<='\uFFFE')) ) {
                                        alt2=1;
                                    }


                                }
                                else if ( ((LA2_5>='\u0000' && LA2_5<='c')||(LA2_5>='e' && LA2_5<='\uFFFE')) ) {
                                    alt2=1;
                                }


                            }
                            else if ( ((LA2_4>='\u0000' && LA2_4<='c')||(LA2_4>='e' && LA2_4<='\uFFFE')) ) {
                                alt2=1;
                            }


                        }
                        else if ( ((LA2_3>='\u0000' && LA2_3<='m')||(LA2_3>='o' && LA2_3<='\uFFFE')) ) {
                            alt2=1;
                        }


                    }
                    else if ( ((LA2_1>='\u0000' && LA2_1<='d')||(LA2_1>='f' && LA2_1<='\uFFFE')) ) {
                        alt2=1;
                    }


                }
                else if ( ((LA2_0>='\u0000' && LA2_0<='\"')||(LA2_0>='$' && LA2_0<='\uFFFE')) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1558:53: .
            	    {
            	    matchAny(); 

            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

            match("#enddef"); 


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_DEFINE

    // $ANTLR start RULE_TEXTDOMAIN
    public final void mRULE_TEXTDOMAIN() throws RecognitionException {
        try {
            int _type = RULE_TEXTDOMAIN;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:17: ( '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:19: '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#textdomain"); 

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:33: (~ ( ( '\\n' | '\\r' ) ) )*
            loop3:
            do {
                int alt3=2;
                int LA3_0 = input.LA(1);

                if ( ((LA3_0>='\u0000' && LA3_0<='\t')||(LA3_0>='\u000B' && LA3_0<='\f')||(LA3_0>='\u000E' && LA3_0<='\uFFFE')) ) {
                    alt3=1;
                }


                switch (alt3) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:33: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFE') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop3;
                }
            } while (true);

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:49: ( ( '\\r' )? '\\n' )?
            int alt5=2;
            int LA5_0 = input.LA(1);

            if ( (LA5_0=='\n'||LA5_0=='\r') ) {
                alt5=1;
            }
            switch (alt5) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:50: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:50: ( '\\r' )?
                    int alt4=2;
                    int LA4_0 = input.LA(1);

                    if ( (LA4_0=='\r') ) {
                        alt4=1;
                    }
                    switch (alt4) {
                        case 1 :
                            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1560:50: '\\r'
                            {
                            match('\r'); 

                            }
                            break;

                    }

                    match('\n'); 

                    }
                    break;

            }


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_TEXTDOMAIN

    // $ANTLR start RULE_STRING
    public final void mRULE_STRING() throws RecognitionException {
        try {
            int _type = RULE_STRING;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:13: ( '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"' )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:15: '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"'
            {
            match('\"'); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:19: ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )*
            loop6:
            do {
                int alt6=3;
                int LA6_0 = input.LA(1);

                if ( (LA6_0=='\\') ) {
                    alt6=1;
                }
                else if ( ((LA6_0>='\u0000' && LA6_0<='!')||(LA6_0>='#' && LA6_0<='[')||(LA6_0>=']' && LA6_0<='\uFFFE')) ) {
                    alt6=2;
                }


                switch (alt6) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:20: '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' )
            	    {
            	    match('\\'); 
            	    if ( input.LA(1)=='\"'||input.LA(1)=='\''||input.LA(1)=='\\'||input.LA(1)=='b'||input.LA(1)=='f'||input.LA(1)=='n'||input.LA(1)=='r'||input.LA(1)=='t' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;
            	case 2 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1562:61: ~ ( ( '\\\\' | '\"' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='!')||(input.LA(1)>='#' && input.LA(1)<='[')||(input.LA(1)>=']' && input.LA(1)<='\uFFFE') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop6;
                }
            } while (true);

            match('\"'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_STRING

    // $ANTLR start RULE_ID
    public final void mRULE_ID() throws RecognitionException {
        try {
            int _type = RULE_ID;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1564:9: ( ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | ',' )+ )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1564:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | ',' )+
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1564:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | ',' )+
            int cnt7=0;
            loop7:
            do {
                int alt7=2;
                int LA7_0 = input.LA(1);

                if ( (LA7_0==','||(LA7_0>='0' && LA7_0<='9')||(LA7_0>='A' && LA7_0<='Z')||LA7_0=='_'||(LA7_0>='a' && LA7_0<='z')) ) {
                    alt7=1;
                }


                switch (alt7) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:
            	    {
            	    if ( input.LA(1)==','||(input.LA(1)>='0' && input.LA(1)<='9')||(input.LA(1)>='A' && input.LA(1)<='Z')||input.LA(1)=='_'||(input.LA(1)>='a' && input.LA(1)<='z') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
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

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_ID

    // $ANTLR start RULE_EOL
    public final void mRULE_EOL() throws RecognitionException {
        try {
            int _type = RULE_EOL;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1566:10: ( ( '\\r' | '\\n' )+ )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1566:12: ( '\\r' | '\\n' )+
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1566:12: ( '\\r' | '\\n' )+
            int cnt8=0;
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( (LA8_0=='\n'||LA8_0=='\r') ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:
            	    {
            	    if ( input.LA(1)=='\n'||input.LA(1)=='\r' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
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


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_EOL

    // $ANTLR start RULE_WS
    public final void mRULE_WS() throws RecognitionException {
        try {
            int _type = RULE_WS;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1568:9: ( ( ' ' | '\\t' )+ )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1568:11: ( ' ' | '\\t' )+
            {
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1568:11: ( ' ' | '\\t' )+
            int cnt9=0;
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( (LA9_0=='\t'||LA9_0==' ') ) {
                    alt9=1;
                }


                switch (alt9) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:
            	    {
            	    if ( input.LA(1)=='\t'||input.LA(1)==' ' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    if ( cnt9 >= 1 ) break loop9;
                        EarlyExitException eee =
                            new EarlyExitException(9, input);
                        throw eee;
                }
                cnt9++;
            } while (true);


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_WS

    // $ANTLR start RULE_ANY_OTHER
    public final void mRULE_ANY_OTHER() throws RecognitionException {
        try {
            int _type = RULE_ANY_OTHER;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1570:16: ( . )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1570:18: .
            {
            matchAny(); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_ANY_OTHER

    // $ANTLR start RULE_SL_COMMENT
    public final void mRULE_SL_COMMENT() throws RecognitionException {
        try {
            int _type = RULE_SL_COMMENT;
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:17: ( '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:19: '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match('#'); 
            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop10:
            do {
                int alt10=2;
                int LA10_0 = input.LA(1);

                if ( ((LA10_0>='\u0000' && LA10_0<='\t')||(LA10_0>='\u000B' && LA10_0<='\f')||(LA10_0>='\u000E' && LA10_0<='\uFFFE')) ) {
                    alt10=1;
                }


                switch (alt10) {
            	case 1 :
            	    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:23: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFE') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse =
            	            new MismatchedSetException(null,input);
            	        recover(mse);    throw mse;
            	    }


            	    }
            	    break;

            	default :
            	    break loop10;
                }
            } while (true);

            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:39: ( ( '\\r' )? '\\n' )?
            int alt12=2;
            int LA12_0 = input.LA(1);

            if ( (LA12_0=='\n'||LA12_0=='\r') ) {
                alt12=1;
            }
            switch (alt12) {
                case 1 :
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:40: ( '\\r' )?
                    int alt11=2;
                    int LA11_0 = input.LA(1);

                    if ( (LA11_0=='\r') ) {
                        alt11=1;
                    }
                    switch (alt11) {
                        case 1 :
                            // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1572:40: '\\r'
                            {
                            match('\r'); 

                            }
                            break;

                    }

                    match('\n'); 

                    }
                    break;

            }


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_SL_COMMENT

    public void mTokens() throws RecognitionException {
        // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:8: ( T13 | T14 | T15 | T16 | T17 | T18 | T19 | T20 | RULE_LUA_CODE | RULE_DEFINE | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT )
        int alt13=17;
        alt13 = dfa13.predict(input);
        switch (alt13) {
            case 1 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:10: T13
                {
                mT13(); 

                }
                break;
            case 2 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:14: T14
                {
                mT14(); 

                }
                break;
            case 3 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:18: T15
                {
                mT15(); 

                }
                break;
            case 4 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:22: T16
                {
                mT16(); 

                }
                break;
            case 5 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:26: T17
                {
                mT17(); 

                }
                break;
            case 6 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:30: T18
                {
                mT18(); 

                }
                break;
            case 7 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:34: T19
                {
                mT19(); 

                }
                break;
            case 8 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:38: T20
                {
                mT20(); 

                }
                break;
            case 9 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:42: RULE_LUA_CODE
                {
                mRULE_LUA_CODE(); 

                }
                break;
            case 10 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:56: RULE_DEFINE
                {
                mRULE_DEFINE(); 

                }
                break;
            case 11 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:68: RULE_TEXTDOMAIN
                {
                mRULE_TEXTDOMAIN(); 

                }
                break;
            case 12 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:84: RULE_STRING
                {
                mRULE_STRING(); 

                }
                break;
            case 13 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:96: RULE_ID
                {
                mRULE_ID(); 

                }
                break;
            case 14 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:104: RULE_EOL
                {
                mRULE_EOL(); 

                }
                break;
            case 15 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:113: RULE_WS
                {
                mRULE_WS(); 

                }
                break;
            case 16 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:121: RULE_ANY_OTHER
                {
                mRULE_ANY_OTHER(); 

                }
                break;
            case 17 :
                // ../org.wesnoth.wml.ui/src-gen/org/wesnoth/ui/contentassist/antlr/internal/InternalWML.g:1:136: RULE_SL_COMMENT
                {
                mRULE_SL_COMMENT(); 

                }
                break;

        }

    }


    protected DFA13 dfa13 = new DFA13(this);
    static final String DFA13_eotS =
        "\2\uffff\1\21\5\uffff\3\16\15\uffff\2\32\5\uffff\13\32\1\uffff\4"+
        "\32\1\uffff\4\32\1\71\1\32\1\71\3\uffff\1\32\1\57";
    static final String DFA13_eofS =
        "\74\uffff";
    static final String DFA13_minS =
        "\1\0\1\uffff\1\57\5\uffff\1\74\2\0\15\uffff\2\145\5\uffff\1\146"+
        "\1\170\1\151\1\164\1\156\1\144\1\145\1\157\1\0\1\155\4\0\1\141\1"+
        "\0\1\uffff\1\151\1\0\1\156\4\0\1\12\2\uffff\2\0";
    static final String DFA13_maxS =
        "\1\ufffe\1\uffff\1\57\5\uffff\1\74\2\ufffe\15\uffff\2\145\5\uffff"+
        "\1\146\1\170\1\151\1\164\1\156\1\144\1\145\1\157\1\ufffe\1\155\4"+
        "\ufffe\1\141\1\ufffe\1\uffff\1\151\1\ufffe\1\156\4\ufffe\1\12\2"+
        "\uffff\2\ufffe";
    static final String DFA13_acceptS =
        "\1\uffff\1\1\1\uffff\1\3\1\5\1\6\1\7\1\10\3\uffff\1\15\1\16\1\17"+
        "\1\20\1\1\1\4\1\2\1\3\1\5\1\6\1\7\1\10\1\11\2\uffff\1\21\1\14\1"+
        "\15\1\16\1\17\20\uffff\1\12\10\uffff\2\13\2\uffff";
    static final String DFA13_specialS =
        "\74\uffff}>";
    static final String[] DFA13_transitionS = {
            "\11\16\1\15\1\14\2\16\1\14\22\16\1\15\1\16\1\12\1\11\7\16\1"+
            "\1\1\13\3\16\12\13\2\16\1\10\1\4\3\16\32\13\1\2\1\16\1\3\1\16"+
            "\1\13\1\16\32\13\1\5\1\16\1\6\1\7\uff80\16",
            "",
            "\1\20",
            "",
            "",
            "",
            "",
            "",
            "\1\27",
            "\144\32\1\30\17\32\1\31\uff8a\32",
            "\uffff\33",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "",
            "\1\37",
            "\1\40",
            "",
            "",
            "",
            "",
            "",
            "\1\41",
            "\1\42",
            "\1\43",
            "\1\44",
            "\1\45",
            "\1\46",
            "\1\47",
            "\1\50",
            "\12\54\1\53\2\54\1\52\25\54\1\51\uffdb\54",
            "\1\55",
            "\12\54\1\53\2\54\1\52\25\54\1\51\101\54\1\56\uff99\54",
            "\12\57\1\53\ufff4\57",
            "\uffff\57",
            "\12\54\1\53\2\54\1\52\25\54\1\51\uffdb\54",
            "\1\60",
            "\12\54\1\53\2\54\1\52\25\54\1\51\112\54\1\61\uff90\54",
            "",
            "\1\62",
            "\12\54\1\53\2\54\1\52\25\54\1\51\100\54\1\63\uff9a\54",
            "\1\64",
            "\12\54\1\53\2\54\1\52\25\54\1\51\100\54\1\65\uff9a\54",
            "\12\66\1\70\2\66\1\67\ufff1\66",
            "\12\54\1\53\2\54\1\52\25\54\1\51\101\54\1\72\uff99\54",
            "\12\66\1\70\2\66\1\67\ufff1\66",
            "\1\70",
            "",
            "",
            "\12\54\1\53\2\54\1\52\25\54\1\51\102\54\1\73\uff98\54",
            "\12\54\1\53\2\54\1\52\25\54\1\51\uffdb\54"
    };

    static final short[] DFA13_eot = DFA.unpackEncodedString(DFA13_eotS);
    static final short[] DFA13_eof = DFA.unpackEncodedString(DFA13_eofS);
    static final char[] DFA13_min = DFA.unpackEncodedStringToUnsignedChars(DFA13_minS);
    static final char[] DFA13_max = DFA.unpackEncodedStringToUnsignedChars(DFA13_maxS);
    static final short[] DFA13_accept = DFA.unpackEncodedString(DFA13_acceptS);
    static final short[] DFA13_special = DFA.unpackEncodedString(DFA13_specialS);
    static final short[][] DFA13_transition;

    static {
        int numStates = DFA13_transitionS.length;
        DFA13_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA13_transition[i] = DFA.unpackEncodedString(DFA13_transitionS[i]);
        }
    }

    class DFA13 extends DFA {

        public DFA13(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 13;
            this.eot = DFA13_eot;
            this.eof = DFA13_eof;
            this.min = DFA13_min;
            this.max = DFA13_max;
            this.accept = DFA13_accept;
            this.special = DFA13_special;
            this.transition = DFA13_transition;
        }
        public String getDescription() {
            return "1:1: Tokens : ( T13 | T14 | T15 | T16 | T17 | T18 | T19 | T20 | RULE_LUA_CODE | RULE_DEFINE | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT );";
        }
    }
 

}