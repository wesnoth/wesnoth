package org.wesnoth.parser.antlr.internal;

// Hack: Use our own Lexer superclass by means of import. 
// Currently there is no other way to specify the superclass for the lexer.
import org.eclipse.xtext.parser.antlr.Lexer;


import org.antlr.runtime.*;
import java.util.Stack;
import java.util.List;
import java.util.ArrayList;

@SuppressWarnings("all")
public class InternalWMLLexer extends Lexer {
    public static final int RULE_ID=4;
    public static final int RULE_ENDDEFINE=8;
    public static final int RULE_ANY_OTHER=6;
    public static final int RULE_TEXTDOMAIN=10;
    public static final int T27=27;
    public static final int T26=26;
    public static final int T25=25;
    public static final int Tokens=28;
    public static final int T24=24;
    public static final int EOF=-1;
    public static final int RULE_SL_COMMENT=11;
    public static final int T23=23;
    public static final int T22=22;
    public static final int T21=21;
    public static final int T20=20;
    public static final int RULE_STRING=5;
    public static final int RULE_DEFINE=7;
    public static final int RULE_INT=9;
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
    public String getGrammarFileName() { return "../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g"; }

    // $ANTLR start T13
    public final void mT13() throws RecognitionException {
        try {
            int _type = T13;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:10:5: ( '[' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:10:7: '['
            {
            match('['); 

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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:11:5: ( '+' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:11:7: '+'
            {
            match('+'); 

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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:12:5: ( ']' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:12:7: ']'
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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:13:5: ( '[/' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:13:7: '[/'
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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:14:5: ( '{' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:14:7: '{'
            {
            match('{'); 

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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:15:5: ( '}' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:15:7: '}'
            {
            match('}'); 

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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:16:5: ( '=' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:16:7: '='
            {
            match('='); 

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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:17:5: ( '(' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:17:7: '('
            {
            match('('); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T20

    // $ANTLR start T21
    public final void mT21() throws RecognitionException {
        try {
            int _type = T21;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:18:5: ( ')' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:18:7: ')'
            {
            match(')'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T21

    // $ANTLR start T22
    public final void mT22() throws RecognitionException {
        try {
            int _type = T22;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:19:5: ( '~' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:19:7: '~'
            {
            match('~'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T22

    // $ANTLR start T23
    public final void mT23() throws RecognitionException {
        try {
            int _type = T23;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:20:5: ( '/' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:20:7: '/'
            {
            match('/'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T23

    // $ANTLR start T24
    public final void mT24() throws RecognitionException {
        try {
            int _type = T24;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:21:5: ( ',' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:21:7: ','
            {
            match(','); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T24

    // $ANTLR start T25
    public final void mT25() throws RecognitionException {
        try {
            int _type = T25;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:22:5: ( '_' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:22:7: '_'
            {
            match('_'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T25

    // $ANTLR start T26
    public final void mT26() throws RecognitionException {
        try {
            int _type = T26;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:23:5: ( '.' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:23:7: '.'
            {
            match('.'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T26

    // $ANTLR start T27
    public final void mT27() throws RecognitionException {
        try {
            int _type = T27;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:24:5: ( '-' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:24:7: '-'
            {
            match('-'); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end T27

    // $ANTLR start RULE_ID
    public final void mRULE_ID() throws RecognitionException {
        try {
            int _type = RULE_ID;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1440:9: ( ( 'a' .. 'z' | 'A' .. 'Z' | '_' | '0' .. '9' )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1440:11: ( 'a' .. 'z' | 'A' .. 'Z' | '_' | '0' .. '9' )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1440:11: ( 'a' .. 'z' | 'A' .. 'Z' | '_' | '0' .. '9' )+
            int cnt1=0;
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( ((LA1_0>='0' && LA1_0<='9')||(LA1_0>='A' && LA1_0<='Z')||LA1_0=='_'||(LA1_0>='a' && LA1_0<='z')) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:
            	    {
            	    if ( (input.LA(1)>='0' && input.LA(1)<='9')||(input.LA(1)>='A' && input.LA(1)<='Z')||input.LA(1)=='_'||(input.LA(1)>='a' && input.LA(1)<='z') ) {
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
            	    if ( cnt1 >= 1 ) break loop1;
                        EarlyExitException eee =
                            new EarlyExitException(1, input);
                        throw eee;
                }
                cnt1++;
            } while (true);


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_ID

    // $ANTLR start RULE_INT
    public final void mRULE_INT() throws RecognitionException {
        try {
            int _type = RULE_INT;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1442:10: ( ( '0' .. '9' )* ( '.' ( '0' .. '9' )+ )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1442:12: ( '0' .. '9' )* ( '.' ( '0' .. '9' )+ )?
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1442:12: ( '0' .. '9' )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>='0' && LA2_0<='9')) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1442:13: '0' .. '9'
            	    {
            	    matchRange('0','9'); 

            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1442:24: ( '.' ( '0' .. '9' )+ )?
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0=='.') ) {
                alt4=1;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1442:25: '.' ( '0' .. '9' )+
                    {
                    match('.'); 
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1442:29: ( '0' .. '9' )+
                    int cnt3=0;
                    loop3:
                    do {
                        int alt3=2;
                        int LA3_0 = input.LA(1);

                        if ( ((LA3_0>='0' && LA3_0<='9')) ) {
                            alt3=1;
                        }


                        switch (alt3) {
                    	case 1 :
                    	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1442:30: '0' .. '9'
                    	    {
                    	    matchRange('0','9'); 

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


                    }
                    break;

            }


            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_INT

    // $ANTLR start RULE_STRING
    public final void mRULE_STRING() throws RecognitionException {
        try {
            int _type = RULE_STRING;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1444:13: ( '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"' )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1444:15: '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"'
            {
            match('\"'); 
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1444:19: ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )*
            loop5:
            do {
                int alt5=3;
                int LA5_0 = input.LA(1);

                if ( (LA5_0=='\\') ) {
                    alt5=1;
                }
                else if ( ((LA5_0>='\u0000' && LA5_0<='!')||(LA5_0>='#' && LA5_0<='[')||(LA5_0>=']' && LA5_0<='\uFFFE')) ) {
                    alt5=2;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1444:20: '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' )
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
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1444:61: ~ ( ( '\\\\' | '\"' ) )
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
            	    break loop5;
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

    // $ANTLR start RULE_TEXTDOMAIN
    public final void mRULE_TEXTDOMAIN() throws RecognitionException {
        try {
            int _type = RULE_TEXTDOMAIN;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:17: ( '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:19: '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#textdomain"); 

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:33: (~ ( ( '\\n' | '\\r' ) ) )*
            loop6:
            do {
                int alt6=2;
                int LA6_0 = input.LA(1);

                if ( ((LA6_0>='\u0000' && LA6_0<='\t')||(LA6_0>='\u000B' && LA6_0<='\f')||(LA6_0>='\u000E' && LA6_0<='\uFFFE')) ) {
                    alt6=1;
                }


                switch (alt6) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:33: ~ ( ( '\\n' | '\\r' ) )
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
            	    break loop6;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:49: ( ( '\\r' )? '\\n' )?
            int alt8=2;
            int LA8_0 = input.LA(1);

            if ( (LA8_0=='\n'||LA8_0=='\r') ) {
                alt8=1;
            }
            switch (alt8) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:50: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:50: ( '\\r' )?
                    int alt7=2;
                    int LA7_0 = input.LA(1);

                    if ( (LA7_0=='\r') ) {
                        alt7=1;
                    }
                    switch (alt7) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1446:50: '\\r'
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

    // $ANTLR start RULE_DEFINE
    public final void mRULE_DEFINE() throws RecognitionException {
        try {
            int _type = RULE_DEFINE;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1448:13: ( '#define' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1448:15: '#define' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#define"); 

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1448:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop9:
            do {
                int alt9=2;
                int LA9_0 = input.LA(1);

                if ( ((LA9_0>='\u0000' && LA9_0<='\t')||(LA9_0>='\u000B' && LA9_0<='\f')||(LA9_0>='\u000E' && LA9_0<='\uFFFE')) ) {
                    alt9=1;
                }


                switch (alt9) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1448:25: ~ ( ( '\\n' | '\\r' ) )
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
            	    break loop9;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1448:41: ( ( '\\r' )? '\\n' )?
            int alt11=2;
            int LA11_0 = input.LA(1);

            if ( (LA11_0=='\n'||LA11_0=='\r') ) {
                alt11=1;
            }
            switch (alt11) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1448:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1448:42: ( '\\r' )?
                    int alt10=2;
                    int LA10_0 = input.LA(1);

                    if ( (LA10_0=='\r') ) {
                        alt10=1;
                    }
                    switch (alt10) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1448:42: '\\r'
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
    // $ANTLR end RULE_DEFINE

    // $ANTLR start RULE_ENDDEFINE
    public final void mRULE_ENDDEFINE() throws RecognitionException {
        try {
            int _type = RULE_ENDDEFINE;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:16: ( '#enddef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:18: '#enddef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#enddef"); 

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:28: (~ ( ( '\\n' | '\\r' ) ) )*
            loop12:
            do {
                int alt12=2;
                int LA12_0 = input.LA(1);

                if ( ((LA12_0>='\u0000' && LA12_0<='\t')||(LA12_0>='\u000B' && LA12_0<='\f')||(LA12_0>='\u000E' && LA12_0<='\uFFFE')) ) {
                    alt12=1;
                }


                switch (alt12) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:28: ~ ( ( '\\n' | '\\r' ) )
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
            	    break loop12;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:44: ( ( '\\r' )? '\\n' )?
            int alt14=2;
            int LA14_0 = input.LA(1);

            if ( (LA14_0=='\n'||LA14_0=='\r') ) {
                alt14=1;
            }
            switch (alt14) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:45: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:45: ( '\\r' )?
                    int alt13=2;
                    int LA13_0 = input.LA(1);

                    if ( (LA13_0=='\r') ) {
                        alt13=1;
                    }
                    switch (alt13) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1450:45: '\\r'
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
    // $ANTLR end RULE_ENDDEFINE

    // $ANTLR start RULE_SL_COMMENT
    public final void mRULE_SL_COMMENT() throws RecognitionException {
        try {
            int _type = RULE_SL_COMMENT;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:17: ( '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:19: '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match('#'); 
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop15:
            do {
                int alt15=2;
                int LA15_0 = input.LA(1);

                if ( ((LA15_0>='\u0000' && LA15_0<='\t')||(LA15_0>='\u000B' && LA15_0<='\f')||(LA15_0>='\u000E' && LA15_0<='\uFFFE')) ) {
                    alt15=1;
                }


                switch (alt15) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:23: ~ ( ( '\\n' | '\\r' ) )
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
            	    break loop15;
                }
            } while (true);

            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:39: ( ( '\\r' )? '\\n' )?
            int alt17=2;
            int LA17_0 = input.LA(1);

            if ( (LA17_0=='\n'||LA17_0=='\r') ) {
                alt17=1;
            }
            switch (alt17) {
                case 1 :
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:40: ( '\\r' )?
                    int alt16=2;
                    int LA16_0 = input.LA(1);

                    if ( (LA16_0=='\r') ) {
                        alt16=1;
                    }
                    switch (alt16) {
                        case 1 :
                            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1452:40: '\\r'
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

    // $ANTLR start RULE_WS
    public final void mRULE_WS() throws RecognitionException {
        try {
            int _type = RULE_WS;
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1454:9: ( ( ' ' | '\\t' | '\\r' | '\\n' )+ )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1454:11: ( ' ' | '\\t' | '\\r' | '\\n' )+
            {
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1454:11: ( ' ' | '\\t' | '\\r' | '\\n' )+
            int cnt18=0;
            loop18:
            do {
                int alt18=2;
                int LA18_0 = input.LA(1);

                if ( ((LA18_0>='\t' && LA18_0<='\n')||LA18_0=='\r'||LA18_0==' ') ) {
                    alt18=1;
                }


                switch (alt18) {
            	case 1 :
            	    // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:
            	    {
            	    if ( (input.LA(1)>='\t' && input.LA(1)<='\n')||input.LA(1)=='\r'||input.LA(1)==' ' ) {
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
            	    if ( cnt18 >= 1 ) break loop18;
                        EarlyExitException eee =
                            new EarlyExitException(18, input);
                        throw eee;
                }
                cnt18++;
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
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1456:16: ( . )
            // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1456:18: .
            {
            matchAny(); 

            }

            this.type = _type;
        }
        finally {
        }
    }
    // $ANTLR end RULE_ANY_OTHER

    public void mTokens() throws RecognitionException {
        // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:8: ( T13 | T14 | T15 | T16 | T17 | T18 | T19 | T20 | T21 | T22 | T23 | T24 | T25 | T26 | T27 | RULE_ID | RULE_INT | RULE_STRING | RULE_TEXTDOMAIN | RULE_DEFINE | RULE_ENDDEFINE | RULE_SL_COMMENT | RULE_WS | RULE_ANY_OTHER )
        int alt19=24;
        alt19 = dfa19.predict(input);
        switch (alt19) {
            case 1 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:10: T13
                {
                mT13(); 

                }
                break;
            case 2 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:14: T14
                {
                mT14(); 

                }
                break;
            case 3 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:18: T15
                {
                mT15(); 

                }
                break;
            case 4 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:22: T16
                {
                mT16(); 

                }
                break;
            case 5 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:26: T17
                {
                mT17(); 

                }
                break;
            case 6 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:30: T18
                {
                mT18(); 

                }
                break;
            case 7 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:34: T19
                {
                mT19(); 

                }
                break;
            case 8 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:38: T20
                {
                mT20(); 

                }
                break;
            case 9 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:42: T21
                {
                mT21(); 

                }
                break;
            case 10 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:46: T22
                {
                mT22(); 

                }
                break;
            case 11 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:50: T23
                {
                mT23(); 

                }
                break;
            case 12 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:54: T24
                {
                mT24(); 

                }
                break;
            case 13 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:58: T25
                {
                mT25(); 

                }
                break;
            case 14 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:62: T26
                {
                mT26(); 

                }
                break;
            case 15 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:66: T27
                {
                mT27(); 

                }
                break;
            case 16 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:70: RULE_ID
                {
                mRULE_ID(); 

                }
                break;
            case 17 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:78: RULE_INT
                {
                mRULE_INT(); 

                }
                break;
            case 18 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:87: RULE_STRING
                {
                mRULE_STRING(); 

                }
                break;
            case 19 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:99: RULE_TEXTDOMAIN
                {
                mRULE_TEXTDOMAIN(); 

                }
                break;
            case 20 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:115: RULE_DEFINE
                {
                mRULE_DEFINE(); 

                }
                break;
            case 21 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:127: RULE_ENDDEFINE
                {
                mRULE_ENDDEFINE(); 

                }
                break;
            case 22 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:142: RULE_SL_COMMENT
                {
                mRULE_SL_COMMENT(); 

                }
                break;
            case 23 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:158: RULE_WS
                {
                mRULE_WS(); 

                }
                break;
            case 24 :
                // ../org.wesnoth.wml/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:166: RULE_ANY_OTHER
                {
                mRULE_ANY_OTHER(); 

                }
                break;

        }

    }


    protected DFA19 dfa19 = new DFA19(this);
    static final String DFA19_eotS =
        "\1\21\1\27\12\uffff\1\42\1\44\1\uffff\1\43\2\uffff\1\25\1\53\22"+
        "\uffff\1\43\1\uffff\3\53\2\uffff\14\53\1\77\1\53\1\103\1\77\3\uffff"+
        "\1\53\3\uffff\1\103\2\53\1\112\3\uffff\1\112";
    static final String DFA19_eofS =
        "\114\uffff";
    static final String DFA19_minS =
        "\1\0\1\57\12\uffff\2\60\1\uffff\1\56\2\uffff\1\0\1\144\22\uffff"+
        "\1\56\1\uffff\1\156\2\145\2\uffff\1\144\1\170\1\146\1\144\1\164"+
        "\1\151\1\145\1\144\1\156\1\146\1\157\1\145\1\0\1\155\2\0\1\12\2"+
        "\uffff\1\141\1\12\2\uffff\1\0\1\151\1\156\1\0\1\12\2\uffff\1\0";
    static final String DFA19_maxS =
        "\1\ufffe\1\57\12\uffff\1\172\1\71\1\uffff\1\71\2\uffff\1\ufffe\1"+
        "\164\22\uffff\1\71\1\uffff\1\156\2\145\2\uffff\1\144\1\170\1\146"+
        "\1\144\1\164\1\151\1\145\1\144\1\156\1\146\1\157\1\145\1\ufffe\1"+
        "\155\2\ufffe\1\12\2\uffff\1\141\1\12\2\uffff\1\ufffe\1\151\1\156"+
        "\1\ufffe\1\12\2\uffff\1\ufffe";
    static final String DFA19_acceptS =
        "\2\uffff\1\2\1\3\1\5\1\6\1\7\1\10\1\11\1\12\1\13\1\14\2\uffff\1"+
        "\17\1\uffff\1\20\1\21\2\uffff\1\27\1\30\1\4\1\1\1\2\1\3\1\5\1\6"+
        "\1\7\1\10\1\11\1\12\1\13\1\14\1\15\1\20\1\16\1\17\1\uffff\1\22\3"+
        "\uffff\1\26\1\27\21\uffff\2\25\2\uffff\2\24\5\uffff\2\23\1\uffff";
    static final String DFA19_specialS =
        "\114\uffff}>";
    static final String[] DFA19_transitionS = {
            "\11\25\2\24\2\25\1\24\22\25\1\24\1\25\1\22\1\23\4\25\1\7\1\10"+
            "\1\25\1\2\1\13\1\16\1\15\1\12\12\17\3\25\1\6\3\25\32\20\1\1"+
            "\1\25\1\3\1\25\1\14\1\25\32\20\1\4\1\25\1\5\1\11\uff80\25",
            "\1\26",
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
            "\12\43\7\uffff\32\43\4\uffff\1\43\1\uffff\32\43",
            "\12\21",
            "",
            "\1\21\1\uffff\12\46",
            "",
            "",
            "\uffff\47",
            "\1\52\1\50\16\uffff\1\51",
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
            "",
            "",
            "",
            "",
            "",
            "\1\21\1\uffff\12\46",
            "",
            "\1\55",
            "\1\56",
            "\1\57",
            "",
            "",
            "\1\60",
            "\1\61",
            "\1\62",
            "\1\63",
            "\1\64",
            "\1\65",
            "\1\66",
            "\1\67",
            "\1\70",
            "\1\71",
            "\1\72",
            "\1\73",
            "\12\74\1\76\2\74\1\75\ufff1\74",
            "\1\100",
            "\12\104\1\102\2\104\1\101\ufff1\104",
            "\12\74\1\76\2\74\1\75\ufff1\74",
            "\1\76",
            "",
            "",
            "\1\105",
            "\1\102",
            "",
            "",
            "\12\104\1\102\2\104\1\101\ufff1\104",
            "\1\106",
            "\1\107",
            "\12\113\1\111\2\113\1\110\ufff1\113",
            "\1\111",
            "",
            "",
            "\12\113\1\111\2\113\1\110\ufff1\113"
    };

    static final short[] DFA19_eot = DFA.unpackEncodedString(DFA19_eotS);
    static final short[] DFA19_eof = DFA.unpackEncodedString(DFA19_eofS);
    static final char[] DFA19_min = DFA.unpackEncodedStringToUnsignedChars(DFA19_minS);
    static final char[] DFA19_max = DFA.unpackEncodedStringToUnsignedChars(DFA19_maxS);
    static final short[] DFA19_accept = DFA.unpackEncodedString(DFA19_acceptS);
    static final short[] DFA19_special = DFA.unpackEncodedString(DFA19_specialS);
    static final short[][] DFA19_transition;

    static {
        int numStates = DFA19_transitionS.length;
        DFA19_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA19_transition[i] = DFA.unpackEncodedString(DFA19_transitionS[i]);
        }
    }

    class DFA19 extends DFA {

        public DFA19(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 19;
            this.eot = DFA19_eot;
            this.eof = DFA19_eof;
            this.min = DFA19_min;
            this.max = DFA19_max;
            this.accept = DFA19_accept;
            this.special = DFA19_special;
            this.transition = DFA19_transition;
        }
        public String getDescription() {
            return "1:1: Tokens : ( T13 | T14 | T15 | T16 | T17 | T18 | T19 | T20 | T21 | T22 | T23 | T24 | T25 | T26 | T27 | RULE_ID | RULE_INT | RULE_STRING | RULE_TEXTDOMAIN | RULE_DEFINE | RULE_ENDDEFINE | RULE_SL_COMMENT | RULE_WS | RULE_ANY_OTHER );";
        }
    }
 

}