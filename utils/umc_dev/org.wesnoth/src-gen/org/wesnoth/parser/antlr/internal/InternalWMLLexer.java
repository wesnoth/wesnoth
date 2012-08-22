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
    public static final int RULE_LUA_CODE=18;
    public static final int RULE_ID=4;
    public static final int RULE_IFDEF=9;
    public static final int T__29=29;
    public static final int T__28=28;
    public static final int T__27=27;
    public static final int T__26=26;
    public static final int T__25=25;
    public static final int T__24=24;
    public static final int T__23=23;
    public static final int T__22=22;
    public static final int RULE_ANY_OTHER=20;
    public static final int RULE_IFNDEF=10;
    public static final int RULE_EOL=5;
    public static final int RULE_TEXTDOMAIN=17;
    public static final int RULE_IFNHAVE=12;
    public static final int RULE_SL_COMMENT=6;
    public static final int EOF=-1;
    public static final int RULE_IFNVER=14;
    public static final int T__30=30;
    public static final int T__31=31;
    public static final int T__32=32;
    public static final int RULE_STRING=19;
    public static final int T__33=33;
    public static final int T__34=34;
    public static final int RULE_ENDIF=16;
    public static final int T__35=35;
    public static final int T__36=36;
    public static final int RULE_DEFINE=7;
    public static final int RULE_IFVER=13;
    public static final int RULE_ENDDEF=8;
    public static final int RULE_IFHAVE=11;
    public static final int RULE_WS=21;
    public static final int RULE_ELSE=15;

    // delegates
    // delegators

    public InternalWMLLexer() {;} 
    public InternalWMLLexer(CharStream input) {
        this(input, new RecognizerSharedState());
    }
    public InternalWMLLexer(CharStream input, RecognizerSharedState state) {
        super(input,state);

    }
    public String getGrammarFileName() { return "../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g"; }

    // $ANTLR start "T__22"
    public final void mT__22() throws RecognitionException {
        try {
            int _type = T__22;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:11:7: ( '[' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:11:9: '['
            {
            match('['); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__22"

    // $ANTLR start "T__23"
    public final void mT__23() throws RecognitionException {
        try {
            int _type = T__23;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:12:7: ( '+' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:12:9: '+'
            {
            match('+'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__23"

    // $ANTLR start "T__24"
    public final void mT__24() throws RecognitionException {
        try {
            int _type = T__24;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:13:7: ( ']' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:13:9: ']'
            {
            match(']'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__24"

    // $ANTLR start "T__25"
    public final void mT__25() throws RecognitionException {
        try {
            int _type = T__25;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:14:7: ( '[/' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:14:9: '[/'
            {
            match("[/"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__25"

    // $ANTLR start "T__26"
    public final void mT__26() throws RecognitionException {
        try {
            int _type = T__26;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:15:7: ( '=' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:15:9: '='
            {
            match('='); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__26"

    // $ANTLR start "T__27"
    public final void mT__27() throws RecognitionException {
        try {
            int _type = T__27;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:16:7: ( '{' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:16:9: '{'
            {
            match('{'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__27"

    // $ANTLR start "T__28"
    public final void mT__28() throws RecognitionException {
        try {
            int _type = T__28;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:17:7: ( './' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:17:9: './'
            {
            match("./"); 


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__28"

    // $ANTLR start "T__29"
    public final void mT__29() throws RecognitionException {
        try {
            int _type = T__29;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:18:7: ( '~' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:18:9: '~'
            {
            match('~'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__29"

    // $ANTLR start "T__30"
    public final void mT__30() throws RecognitionException {
        try {
            int _type = T__30;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:19:7: ( '}' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:19:9: '}'
            {
            match('}'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__30"

    // $ANTLR start "T__31"
    public final void mT__31() throws RecognitionException {
        try {
            int _type = T__31;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:20:7: ( '_' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:20:9: '_'
            {
            match('_'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__31"

    // $ANTLR start "T__32"
    public final void mT__32() throws RecognitionException {
        try {
            int _type = T__32;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:21:7: ( '.' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:21:9: '.'
            {
            match('.'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__32"

    // $ANTLR start "T__33"
    public final void mT__33() throws RecognitionException {
        try {
            int _type = T__33;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:22:7: ( '$' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:22:9: '$'
            {
            match('$'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__33"

    // $ANTLR start "T__34"
    public final void mT__34() throws RecognitionException {
        try {
            int _type = T__34;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:23:7: ( '/' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:23:9: '/'
            {
            match('/'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__34"

    // $ANTLR start "T__35"
    public final void mT__35() throws RecognitionException {
        try {
            int _type = T__35;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:24:7: ( '(' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:24:9: '('
            {
            match('('); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__35"

    // $ANTLR start "T__36"
    public final void mT__36() throws RecognitionException {
        try {
            int _type = T__36;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:25:7: ( ')' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:25:9: ')'
            {
            match(')'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "T__36"

    // $ANTLR start "RULE_LUA_CODE"
    public final void mRULE_LUA_CODE() throws RecognitionException {
        try {
            int _type = RULE_LUA_CODE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:15: ( '<<' ( options {greedy=false; } : . )* '>>' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:17: '<<' ( options {greedy=false; } : . )* '>>'
            {
            match("<<"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:22: ( options {greedy=false; } : . )*
            loop1:
            do {
                int alt1=2;
                int LA1_0 = input.LA(1);

                if ( (LA1_0=='>') ) {
                    int LA1_1 = input.LA(2);

                    if ( (LA1_1=='>') ) {
                        alt1=2;
                    }
                    else if ( ((LA1_1>='\u0000' && LA1_1<='=')||(LA1_1>='?' && LA1_1<='\uFFFF')) ) {
                        alt1=1;
                    }


                }
                else if ( ((LA1_0>='\u0000' && LA1_0<='=')||(LA1_0>='?' && LA1_0<='\uFFFF')) ) {
                    alt1=1;
                }


                switch (alt1) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:50: .
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_LUA_CODE"

    // $ANTLR start "RULE_IFHAVE"
    public final void mRULE_IFHAVE() throws RecognitionException {
        try {
            int _type = RULE_IFHAVE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:13: ( '#ifhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:15: '#ifhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifhave"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>='\u0000' && LA2_0<='\t')||(LA2_0>='\u000B' && LA2_0<='\f')||(LA2_0>='\u000E' && LA2_0<='\uFFFF')) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:25: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop2;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:41: ( ( '\\r' )? '\\n' )?
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0=='\n'||LA4_0=='\r') ) {
                alt4=1;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:42: ( '\\r' )?
                    int alt3=2;
                    int LA3_0 = input.LA(1);

                    if ( (LA3_0=='\r') ) {
                        alt3=1;
                    }
                    switch (alt3) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1381:42: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_IFHAVE"

    // $ANTLR start "RULE_IFNHAVE"
    public final void mRULE_IFNHAVE() throws RecognitionException {
        try {
            int _type = RULE_IFNHAVE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:14: ( '#ifnhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:16: '#ifnhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifnhave"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:27: (~ ( ( '\\n' | '\\r' ) ) )*
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( ((LA5_0>='\u0000' && LA5_0<='\t')||(LA5_0>='\u000B' && LA5_0<='\f')||(LA5_0>='\u000E' && LA5_0<='\uFFFF')) ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:27: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop5;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:43: ( ( '\\r' )? '\\n' )?
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0=='\n'||LA7_0=='\r') ) {
                alt7=1;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:44: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:44: ( '\\r' )?
                    int alt6=2;
                    int LA6_0 = input.LA(1);

                    if ( (LA6_0=='\r') ) {
                        alt6=1;
                    }
                    switch (alt6) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1383:44: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_IFNHAVE"

    // $ANTLR start "RULE_IFDEF"
    public final void mRULE_IFDEF() throws RecognitionException {
        try {
            int _type = RULE_IFDEF;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1385:12: ( '#ifdef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1385:14: '#ifdef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifdef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1385:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( ((LA8_0>='\u0000' && LA8_0<='\t')||(LA8_0>='\u000B' && LA8_0<='\f')||(LA8_0>='\u000E' && LA8_0<='\uFFFF')) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1385:23: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop8;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1385:39: ( ( '\\r' )? '\\n' )?
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0=='\n'||LA10_0=='\r') ) {
                alt10=1;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1385:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1385:40: ( '\\r' )?
                    int alt9=2;
                    int LA9_0 = input.LA(1);

                    if ( (LA9_0=='\r') ) {
                        alt9=1;
                    }
                    switch (alt9) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1385:40: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_IFDEF"

    // $ANTLR start "RULE_IFVER"
    public final void mRULE_IFVER() throws RecognitionException {
        try {
            int _type = RULE_IFVER;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:12: ( '#ifver' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:14: '#ifver' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifver"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop11:
            do {
                int alt11=2;
                int LA11_0 = input.LA(1);

                if ( ((LA11_0>='\u0000' && LA11_0<='\t')||(LA11_0>='\u000B' && LA11_0<='\f')||(LA11_0>='\u000E' && LA11_0<='\uFFFF')) ) {
                    alt11=1;
                }


                switch (alt11) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:23: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop11;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:39: ( ( '\\r' )? '\\n' )?
            int alt13=2;
            int LA13_0 = input.LA(1);

            if ( (LA13_0=='\n'||LA13_0=='\r') ) {
                alt13=1;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:40: ( '\\r' )?
                    int alt12=2;
                    int LA12_0 = input.LA(1);

                    if ( (LA12_0=='\r') ) {
                        alt12=1;
                    }
                    switch (alt12) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1387:40: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_IFVER"

    // $ANTLR start "RULE_IFNVER"
    public final void mRULE_IFNVER() throws RecognitionException {
        try {
            int _type = RULE_IFNVER;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:13: ( '#ifnver' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:15: '#ifnver' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifnver"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>='\u0000' && LA14_0<='\t')||(LA14_0>='\u000B' && LA14_0<='\f')||(LA14_0>='\u000E' && LA14_0<='\uFFFF')) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:25: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop14;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:41: ( ( '\\r' )? '\\n' )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0=='\n'||LA16_0=='\r') ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:42: ( '\\r' )?
                    int alt15=2;
                    int LA15_0 = input.LA(1);

                    if ( (LA15_0=='\r') ) {
                        alt15=1;
                    }
                    switch (alt15) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1389:42: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_IFNVER"

    // $ANTLR start "RULE_IFNDEF"
    public final void mRULE_IFNDEF() throws RecognitionException {
        try {
            int _type = RULE_IFNDEF;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1391:13: ( '#ifndef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1391:15: '#ifndef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifndef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1391:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop17:
            do {
                int alt17=2;
                int LA17_0 = input.LA(1);

                if ( ((LA17_0>='\u0000' && LA17_0<='\t')||(LA17_0>='\u000B' && LA17_0<='\f')||(LA17_0>='\u000E' && LA17_0<='\uFFFF')) ) {
                    alt17=1;
                }


                switch (alt17) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1391:25: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop17;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1391:41: ( ( '\\r' )? '\\n' )?
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0=='\n'||LA19_0=='\r') ) {
                alt19=1;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1391:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1391:42: ( '\\r' )?
                    int alt18=2;
                    int LA18_0 = input.LA(1);

                    if ( (LA18_0=='\r') ) {
                        alt18=1;
                    }
                    switch (alt18) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1391:42: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_IFNDEF"

    // $ANTLR start "RULE_ELSE"
    public final void mRULE_ELSE() throws RecognitionException {
        try {
            int _type = RULE_ELSE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1393:11: ( '#else' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1393:13: '#else' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#else"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1393:21: (~ ( ( '\\n' | '\\r' ) ) )*
            loop20:
            do {
                int alt20=2;
                int LA20_0 = input.LA(1);

                if ( ((LA20_0>='\u0000' && LA20_0<='\t')||(LA20_0>='\u000B' && LA20_0<='\f')||(LA20_0>='\u000E' && LA20_0<='\uFFFF')) ) {
                    alt20=1;
                }


                switch (alt20) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1393:21: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop20;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1393:37: ( ( '\\r' )? '\\n' )?
            int alt22=2;
            int LA22_0 = input.LA(1);

            if ( (LA22_0=='\n'||LA22_0=='\r') ) {
                alt22=1;
            }
            switch (alt22) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1393:38: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1393:38: ( '\\r' )?
                    int alt21=2;
                    int LA21_0 = input.LA(1);

                    if ( (LA21_0=='\r') ) {
                        alt21=1;
                    }
                    switch (alt21) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1393:38: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_ELSE"

    // $ANTLR start "RULE_ENDIF"
    public final void mRULE_ENDIF() throws RecognitionException {
        try {
            int _type = RULE_ENDIF;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:12: ( '#endif' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:14: '#endif' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#endif"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop23:
            do {
                int alt23=2;
                int LA23_0 = input.LA(1);

                if ( ((LA23_0>='\u0000' && LA23_0<='\t')||(LA23_0>='\u000B' && LA23_0<='\f')||(LA23_0>='\u000E' && LA23_0<='\uFFFF')) ) {
                    alt23=1;
                }


                switch (alt23) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:23: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop23;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:39: ( ( '\\r' )? '\\n' )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0=='\n'||LA25_0=='\r') ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:40: ( '\\r' )?
                    int alt24=2;
                    int LA24_0 = input.LA(1);

                    if ( (LA24_0=='\r') ) {
                        alt24=1;
                    }
                    switch (alt24) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1395:40: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_ENDIF"

    // $ANTLR start "RULE_DEFINE"
    public final void mRULE_DEFINE() throws RecognitionException {
        try {
            int _type = RULE_DEFINE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:13: ( '#define' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:15: '#define' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#define"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop26:
            do {
                int alt26=2;
                int LA26_0 = input.LA(1);

                if ( ((LA26_0>='\u0000' && LA26_0<='\t')||(LA26_0>='\u000B' && LA26_0<='\f')||(LA26_0>='\u000E' && LA26_0<='\uFFFF')) ) {
                    alt26=1;
                }


                switch (alt26) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:25: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop26;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:41: ( ( '\\r' )? '\\n' )?
            int alt28=2;
            int LA28_0 = input.LA(1);

            if ( (LA28_0=='\n'||LA28_0=='\r') ) {
                alt28=1;
            }
            switch (alt28) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:42: ( '\\r' )?
                    int alt27=2;
                    int LA27_0 = input.LA(1);

                    if ( (LA27_0=='\r') ) {
                        alt27=1;
                    }
                    switch (alt27) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1397:42: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_DEFINE"

    // $ANTLR start "RULE_ENDDEF"
    public final void mRULE_ENDDEF() throws RecognitionException {
        try {
            int _type = RULE_ENDDEF;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1399:13: ( '#enddef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1399:15: '#enddef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#enddef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1399:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop29:
            do {
                int alt29=2;
                int LA29_0 = input.LA(1);

                if ( ((LA29_0>='\u0000' && LA29_0<='\t')||(LA29_0>='\u000B' && LA29_0<='\f')||(LA29_0>='\u000E' && LA29_0<='\uFFFF')) ) {
                    alt29=1;
                }


                switch (alt29) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1399:25: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop29;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1399:41: ( ( '\\r' )? '\\n' )?
            int alt31=2;
            int LA31_0 = input.LA(1);

            if ( (LA31_0=='\n'||LA31_0=='\r') ) {
                alt31=1;
            }
            switch (alt31) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1399:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1399:42: ( '\\r' )?
                    int alt30=2;
                    int LA30_0 = input.LA(1);

                    if ( (LA30_0=='\r') ) {
                        alt30=1;
                    }
                    switch (alt30) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1399:42: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_ENDDEF"

    // $ANTLR start "RULE_TEXTDOMAIN"
    public final void mRULE_TEXTDOMAIN() throws RecognitionException {
        try {
            int _type = RULE_TEXTDOMAIN;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:17: ( '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:19: '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#textdomain"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:33: (~ ( ( '\\n' | '\\r' ) ) )*
            loop32:
            do {
                int alt32=2;
                int LA32_0 = input.LA(1);

                if ( ((LA32_0>='\u0000' && LA32_0<='\t')||(LA32_0>='\u000B' && LA32_0<='\f')||(LA32_0>='\u000E' && LA32_0<='\uFFFF')) ) {
                    alt32=1;
                }


                switch (alt32) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:33: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop32;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:49: ( ( '\\r' )? '\\n' )?
            int alt34=2;
            int LA34_0 = input.LA(1);

            if ( (LA34_0=='\n'||LA34_0=='\r') ) {
                alt34=1;
            }
            switch (alt34) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:50: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:50: ( '\\r' )?
                    int alt33=2;
                    int LA33_0 = input.LA(1);

                    if ( (LA33_0=='\r') ) {
                        alt33=1;
                    }
                    switch (alt33) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1401:50: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_TEXTDOMAIN"

    // $ANTLR start "RULE_STRING"
    public final void mRULE_STRING() throws RecognitionException {
        try {
            int _type = RULE_STRING;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1403:13: ( '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1403:15: '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"'
            {
            match('\"'); 
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1403:19: ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )*
            loop35:
            do {
                int alt35=3;
                int LA35_0 = input.LA(1);

                if ( (LA35_0=='\\') ) {
                    alt35=1;
                }
                else if ( ((LA35_0>='\u0000' && LA35_0<='!')||(LA35_0>='#' && LA35_0<='[')||(LA35_0>=']' && LA35_0<='\uFFFF')) ) {
                    alt35=2;
                }


                switch (alt35) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1403:20: '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' )
            	    {
            	    match('\\'); 
            	    if ( input.LA(1)=='\"'||input.LA(1)=='\''||input.LA(1)=='\\'||input.LA(1)=='b'||input.LA(1)=='f'||input.LA(1)=='n'||input.LA(1)=='r'||input.LA(1)=='t' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;
            	case 2 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1403:61: ~ ( ( '\\\\' | '\"' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='!')||(input.LA(1)>='#' && input.LA(1)<='[')||(input.LA(1)>=']' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop35;
                }
            } while (true);

            match('\"'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_STRING"

    // $ANTLR start "RULE_ID"
    public final void mRULE_ID() throws RecognitionException {
        try {
            int _type = RULE_ID;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1405:9: ( ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+ )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1405:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1405:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+
            int cnt36=0;
            loop36:
            do {
                int alt36=2;
                int LA36_0 = input.LA(1);

                if ( ((LA36_0>=',' && LA36_0<='-')||(LA36_0>='0' && LA36_0<='9')||(LA36_0>='A' && LA36_0<='Z')||LA36_0=='_'||(LA36_0>='a' && LA36_0<='z')) ) {
                    alt36=1;
                }


                switch (alt36) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:
            	    {
            	    if ( (input.LA(1)>=',' && input.LA(1)<='-')||(input.LA(1)>='0' && input.LA(1)<='9')||(input.LA(1)>='A' && input.LA(1)<='Z')||input.LA(1)=='_'||(input.LA(1)>='a' && input.LA(1)<='z') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    if ( cnt36 >= 1 ) break loop36;
                        EarlyExitException eee =
                            new EarlyExitException(36, input);
                        throw eee;
                }
                cnt36++;
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_ID"

    // $ANTLR start "RULE_EOL"
    public final void mRULE_EOL() throws RecognitionException {
        try {
            int _type = RULE_EOL;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1407:10: ( ( '\\r' )? '\\n' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1407:12: ( '\\r' )? '\\n'
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1407:12: ( '\\r' )?
            int alt37=2;
            int LA37_0 = input.LA(1);

            if ( (LA37_0=='\r') ) {
                alt37=1;
            }
            switch (alt37) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1407:12: '\\r'
                    {
                    match('\r'); 

                    }
                    break;

            }

            match('\n'); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_EOL"

    // $ANTLR start "RULE_WS"
    public final void mRULE_WS() throws RecognitionException {
        try {
            int _type = RULE_WS;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1409:9: ( ( ' ' | '\\t' )+ )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1409:11: ( ' ' | '\\t' )+
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1409:11: ( ' ' | '\\t' )+
            int cnt38=0;
            loop38:
            do {
                int alt38=2;
                int LA38_0 = input.LA(1);

                if ( (LA38_0=='\t'||LA38_0==' ') ) {
                    alt38=1;
                }


                switch (alt38) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:
            	    {
            	    if ( input.LA(1)=='\t'||input.LA(1)==' ' ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    if ( cnt38 >= 1 ) break loop38;
                        EarlyExitException eee =
                            new EarlyExitException(38, input);
                        throw eee;
                }
                cnt38++;
            } while (true);


            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_WS"

    // $ANTLR start "RULE_ANY_OTHER"
    public final void mRULE_ANY_OTHER() throws RecognitionException {
        try {
            int _type = RULE_ANY_OTHER;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1411:16: ( . )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1411:18: .
            {
            matchAny(); 

            }

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_ANY_OTHER"

    // $ANTLR start "RULE_SL_COMMENT"
    public final void mRULE_SL_COMMENT() throws RecognitionException {
        try {
            int _type = RULE_SL_COMMENT;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:17: ( '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:19: '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match('#'); 
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop39:
            do {
                int alt39=2;
                int LA39_0 = input.LA(1);

                if ( ((LA39_0>='\u0000' && LA39_0<='\t')||(LA39_0>='\u000B' && LA39_0<='\f')||(LA39_0>='\u000E' && LA39_0<='\uFFFF')) ) {
                    alt39=1;
                }


                switch (alt39) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:23: ~ ( ( '\\n' | '\\r' ) )
            	    {
            	    if ( (input.LA(1)>='\u0000' && input.LA(1)<='\t')||(input.LA(1)>='\u000B' && input.LA(1)<='\f')||(input.LA(1)>='\u000E' && input.LA(1)<='\uFFFF') ) {
            	        input.consume();

            	    }
            	    else {
            	        MismatchedSetException mse = new MismatchedSetException(null,input);
            	        recover(mse);
            	        throw mse;}


            	    }
            	    break;

            	default :
            	    break loop39;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:39: ( ( '\\r' )? '\\n' )?
            int alt41=2;
            int LA41_0 = input.LA(1);

            if ( (LA41_0=='\n'||LA41_0=='\r') ) {
                alt41=1;
            }
            switch (alt41) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:40: ( '\\r' )?
                    int alt40=2;
                    int LA40_0 = input.LA(1);

                    if ( (LA40_0=='\r') ) {
                        alt40=1;
                    }
                    switch (alt40) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1413:40: '\\r'
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

            state.type = _type;
            state.channel = _channel;
        }
        finally {
        }
    }
    // $ANTLR end "RULE_SL_COMMENT"

    public void mTokens() throws RecognitionException {
        // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:8: ( T__22 | T__23 | T__24 | T__25 | T__26 | T__27 | T__28 | T__29 | T__30 | T__31 | T__32 | T__33 | T__34 | T__35 | T__36 | RULE_LUA_CODE | RULE_IFHAVE | RULE_IFNHAVE | RULE_IFDEF | RULE_IFVER | RULE_IFNVER | RULE_IFNDEF | RULE_ELSE | RULE_ENDIF | RULE_DEFINE | RULE_ENDDEF | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT )
        int alt42=33;
        alt42 = dfa42.predict(input);
        switch (alt42) {
            case 1 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:10: T__22
                {
                mT__22(); 

                }
                break;
            case 2 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:16: T__23
                {
                mT__23(); 

                }
                break;
            case 3 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:22: T__24
                {
                mT__24(); 

                }
                break;
            case 4 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:28: T__25
                {
                mT__25(); 

                }
                break;
            case 5 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:34: T__26
                {
                mT__26(); 

                }
                break;
            case 6 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:40: T__27
                {
                mT__27(); 

                }
                break;
            case 7 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:46: T__28
                {
                mT__28(); 

                }
                break;
            case 8 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:52: T__29
                {
                mT__29(); 

                }
                break;
            case 9 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:58: T__30
                {
                mT__30(); 

                }
                break;
            case 10 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:64: T__31
                {
                mT__31(); 

                }
                break;
            case 11 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:70: T__32
                {
                mT__32(); 

                }
                break;
            case 12 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:76: T__33
                {
                mT__33(); 

                }
                break;
            case 13 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:82: T__34
                {
                mT__34(); 

                }
                break;
            case 14 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:88: T__35
                {
                mT__35(); 

                }
                break;
            case 15 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:94: T__36
                {
                mT__36(); 

                }
                break;
            case 16 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:100: RULE_LUA_CODE
                {
                mRULE_LUA_CODE(); 

                }
                break;
            case 17 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:114: RULE_IFHAVE
                {
                mRULE_IFHAVE(); 

                }
                break;
            case 18 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:126: RULE_IFNHAVE
                {
                mRULE_IFNHAVE(); 

                }
                break;
            case 19 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:139: RULE_IFDEF
                {
                mRULE_IFDEF(); 

                }
                break;
            case 20 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:150: RULE_IFVER
                {
                mRULE_IFVER(); 

                }
                break;
            case 21 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:161: RULE_IFNVER
                {
                mRULE_IFNVER(); 

                }
                break;
            case 22 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:173: RULE_IFNDEF
                {
                mRULE_IFNDEF(); 

                }
                break;
            case 23 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:185: RULE_ELSE
                {
                mRULE_ELSE(); 

                }
                break;
            case 24 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:195: RULE_ENDIF
                {
                mRULE_ENDIF(); 

                }
                break;
            case 25 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:206: RULE_DEFINE
                {
                mRULE_DEFINE(); 

                }
                break;
            case 26 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:218: RULE_ENDDEF
                {
                mRULE_ENDDEF(); 

                }
                break;
            case 27 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:230: RULE_TEXTDOMAIN
                {
                mRULE_TEXTDOMAIN(); 

                }
                break;
            case 28 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:246: RULE_STRING
                {
                mRULE_STRING(); 

                }
                break;
            case 29 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:258: RULE_ID
                {
                mRULE_ID(); 

                }
                break;
            case 30 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:266: RULE_EOL
                {
                mRULE_EOL(); 

                }
                break;
            case 31 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:275: RULE_WS
                {
                mRULE_WS(); 

                }
                break;
            case 32 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:283: RULE_ANY_OTHER
                {
                mRULE_ANY_OTHER(); 

                }
                break;
            case 33 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:298: RULE_SL_COMMENT
                {
                mRULE_SL_COMMENT(); 

                }
                break;

        }

    }


    protected DFA42 dfa42 = new DFA42(this);
    static final String DFA42_eotS =
        "\1\uffff\1\27\4\uffff\1\35\2\uffff\1\40\4\uffff\3\25\1\uffff\1"+
        "\25\24\uffff\4\53\4\uffff\23\53\1\120\10\53\1\134\1\137\1\120\3"+
        "\uffff\1\143\3\53\1\153\1\53\1\157\1\163\1\134\6\uffff\1\137\3\uffff"+
        "\1\143\1\167\1\173\1\53\1\153\3\uffff\1\u0081\3\uffff\1\157\3\uffff"+
        "\1\163\3\uffff\1\167\3\uffff\1\173\1\53\1\u0081\3\uffff\2\53\2\u0088"+
        "\3\uffff";
    static final String DFA42_eofS =
        "\u0089\uffff";
    static final String DFA42_minS =
        "\1\0\1\57\4\uffff\1\57\2\uffff\1\54\4\uffff\1\74\2\0\1\uffff\1"+
        "\12\24\uffff\1\146\1\154\2\145\4\uffff\1\144\1\163\1\144\1\146\1"+
        "\170\1\141\1\144\3\145\1\144\1\151\1\164\1\166\1\141\2\145\1\146"+
        "\1\162\1\0\1\146\1\145\1\156\1\144\1\145\1\166\1\162\1\146\3\0\1"+
        "\12\2\uffff\1\0\1\146\1\145\1\157\1\0\1\145\3\0\1\12\2\uffff\1\12"+
        "\2\uffff\1\0\1\12\2\uffff\3\0\1\155\1\0\1\12\2\uffff\1\0\1\12\2"+
        "\uffff\1\0\1\12\2\uffff\1\0\1\12\2\uffff\1\0\1\12\2\uffff\1\0\1"+
        "\141\1\0\1\12\2\uffff\1\151\1\156\2\0\1\12\2\uffff";
    static final String DFA42_maxS =
        "\1\uffff\1\57\4\uffff\1\57\2\uffff\1\172\4\uffff\1\74\2\uffff\1"+
        "\uffff\1\12\24\uffff\1\146\1\156\2\145\4\uffff\1\166\1\163\1\144"+
        "\1\146\1\170\1\141\1\166\3\145\2\151\1\164\1\166\1\141\2\145\1\146"+
        "\1\162\1\uffff\1\146\1\145\1\156\1\144\1\145\1\166\1\162\1\146\3"+
        "\uffff\1\12\2\uffff\1\uffff\1\146\1\145\1\157\1\uffff\1\145\3\uffff"+
        "\1\12\2\uffff\1\12\2\uffff\1\uffff\1\12\2\uffff\3\uffff\1\155\1"+
        "\uffff\1\12\2\uffff\1\uffff\1\12\2\uffff\1\uffff\1\12\2\uffff\1"+
        "\uffff\1\12\2\uffff\1\uffff\1\12\2\uffff\1\uffff\1\141\1\uffff\1"+
        "\12\2\uffff\1\151\1\156\2\uffff\1\12\2\uffff";
    static final String DFA42_acceptS =
        "\2\uffff\1\2\1\3\1\5\1\6\1\uffff\1\10\1\11\1\uffff\1\14\1\15\1"+
        "\16\1\17\3\uffff\1\35\1\uffff\1\36\1\37\1\40\1\4\1\1\1\2\1\3\1\5"+
        "\1\6\1\7\1\13\1\10\1\11\1\12\1\35\1\14\1\15\1\16\1\17\1\20\4\uffff"+
        "\1\41\1\34\1\36\1\37\40\uffff\2\27\12\uffff\2\23\1\uffff\2\24\2"+
        "\uffff\2\30\6\uffff\2\21\2\uffff\2\25\2\uffff\2\26\2\uffff\2\32"+
        "\2\uffff\2\31\4\uffff\2\22\5\uffff\2\33";
    static final String DFA42_specialS =
        "\1\7\16\uffff\1\3\1\14\61\uffff\1\10\10\uffff\1\20\1\0\1\30\3\uffff"+
        "\1\16\3\uffff\1\27\1\uffff\1\5\1\22\1\15\6\uffff\1\2\3\uffff\1\17"+
        "\1\25\1\4\1\uffff\1\1\3\uffff\1\11\3\uffff\1\21\3\uffff\1\26\3\uffff"+
        "\1\6\3\uffff\1\13\1\uffff\1\24\5\uffff\1\12\1\23\3\uffff}>";
    static final String[] DFA42_transitionS = {
            "\11\25\1\24\1\23\2\25\1\22\22\25\1\24\1\25\1\20\1\17\1\12\3"+
            "\25\1\14\1\15\1\25\1\2\2\21\1\6\1\13\12\21\2\25\1\16\1\4\3\25"+
            "\32\21\1\1\1\25\1\3\1\25\1\11\1\25\32\21\1\5\1\25\1\10\1\7\uff81"+
            "\25",
            "\1\26",
            "",
            "",
            "",
            "",
            "\1\34",
            "",
            "",
            "\2\41\2\uffff\12\41\7\uffff\32\41\4\uffff\1\41\1\uffff\32"+
            "\41",
            "",
            "",
            "",
            "",
            "\1\46",
            "\144\53\1\51\1\50\3\53\1\47\12\53\1\52\uff8b\53",
            "\0\54",
            "",
            "\1\55",
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
            "",
            "",
            "\1\57",
            "\1\60\1\uffff\1\61",
            "\1\62",
            "\1\63",
            "",
            "",
            "",
            "",
            "\1\66\3\uffff\1\64\5\uffff\1\65\7\uffff\1\67",
            "\1\70",
            "\1\71",
            "\1\72",
            "\1\73",
            "\1\74",
            "\1\77\3\uffff\1\75\15\uffff\1\76",
            "\1\100",
            "\1\101",
            "\1\102",
            "\1\104\4\uffff\1\103",
            "\1\105",
            "\1\106",
            "\1\107",
            "\1\110",
            "\1\111",
            "\1\112",
            "\1\113",
            "\1\114",
            "\12\115\1\117\2\115\1\116\ufff2\115",
            "\1\121",
            "\1\122",
            "\1\123",
            "\1\124",
            "\1\125",
            "\1\126",
            "\1\127",
            "\1\130",
            "\12\131\1\133\2\131\1\132\ufff2\131",
            "\12\140\1\136\2\140\1\135\ufff2\140",
            "\12\115\1\117\2\115\1\116\ufff2\115",
            "\1\117",
            "",
            "",
            "\12\144\1\142\2\144\1\141\ufff2\144",
            "\1\145",
            "\1\146",
            "\1\147",
            "\12\150\1\152\2\150\1\151\ufff2\150",
            "\1\154",
            "\12\160\1\156\2\160\1\155\ufff2\160",
            "\12\164\1\162\2\164\1\161\ufff2\164",
            "\12\131\1\133\2\131\1\132\ufff2\131",
            "\1\133",
            "",
            "",
            "\1\136",
            "",
            "",
            "\12\140\1\136\2\140\1\135\ufff2\140",
            "\1\142",
            "",
            "",
            "\12\144\1\142\2\144\1\141\ufff2\144",
            "\12\170\1\166\2\170\1\165\ufff2\170",
            "\12\174\1\172\2\174\1\171\ufff2\174",
            "\1\175",
            "\12\150\1\152\2\150\1\151\ufff2\150",
            "\1\152",
            "",
            "",
            "\12\176\1\u0080\2\176\1\177\ufff2\176",
            "\1\156",
            "",
            "",
            "\12\160\1\156\2\160\1\155\ufff2\160",
            "\1\162",
            "",
            "",
            "\12\164\1\162\2\164\1\161\ufff2\164",
            "\1\166",
            "",
            "",
            "\12\170\1\166\2\170\1\165\ufff2\170",
            "\1\172",
            "",
            "",
            "\12\174\1\172\2\174\1\171\ufff2\174",
            "\1\u0082",
            "\12\176\1\u0080\2\176\1\177\ufff2\176",
            "\1\u0080",
            "",
            "",
            "\1\u0083",
            "\1\u0084",
            "\12\u0085\1\u0087\2\u0085\1\u0086\ufff2\u0085",
            "\12\u0085\1\u0087\2\u0085\1\u0086\ufff2\u0085",
            "\1\u0087",
            "",
            ""
    };

    static final short[] DFA42_eot = DFA.unpackEncodedString(DFA42_eotS);
    static final short[] DFA42_eof = DFA.unpackEncodedString(DFA42_eofS);
    static final char[] DFA42_min = DFA.unpackEncodedStringToUnsignedChars(DFA42_minS);
    static final char[] DFA42_max = DFA.unpackEncodedStringToUnsignedChars(DFA42_maxS);
    static final short[] DFA42_accept = DFA.unpackEncodedString(DFA42_acceptS);
    static final short[] DFA42_special = DFA.unpackEncodedString(DFA42_specialS);
    static final short[][] DFA42_transition;

    static {
        int numStates = DFA42_transitionS.length;
        DFA42_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA42_transition[i] = DFA.unpackEncodedString(DFA42_transitionS[i]);
        }
    }

    class DFA42 extends DFA {

        public DFA42(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 42;
            this.eot = DFA42_eot;
            this.eof = DFA42_eof;
            this.min = DFA42_min;
            this.max = DFA42_max;
            this.accept = DFA42_accept;
            this.special = DFA42_special;
            this.transition = DFA42_transition;
        }
        public String getDescription() {
            return "1:1: Tokens : ( T__22 | T__23 | T__24 | T__25 | T__26 | T__27 | T__28 | T__29 | T__30 | T__31 | T__32 | T__33 | T__34 | T__35 | T__36 | RULE_LUA_CODE | RULE_IFHAVE | RULE_IFNHAVE | RULE_IFDEF | RULE_IFVER | RULE_IFNVER | RULE_IFNDEF | RULE_ELSE | RULE_ENDIF | RULE_DEFINE | RULE_ENDDEF | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT );";
        }
        public int specialStateTransition(int s, IntStream _input) throws NoViableAltException {
            IntStream input = _input;
        	int _s = s;
            switch ( s ) {
                    case 0 : 
                        int LA42_76 = input.LA(1);

                        s = -1;
                        if ( (LA42_76=='\r') ) {s = 93;}

                        else if ( (LA42_76=='\n') ) {s = 94;}

                        else if ( ((LA42_76>='\u0000' && LA42_76<='\t')||(LA42_76>='\u000B' && LA42_76<='\f')||(LA42_76>='\u000E' && LA42_76<='\uFFFF')) ) {s = 96;}

                        else s = 95;

                        if ( s>=0 ) return s;
                        break;
                    case 1 : 
                        int LA42_104 = input.LA(1);

                        s = -1;
                        if ( (LA42_104=='\r') ) {s = 105;}

                        else if ( (LA42_104=='\n') ) {s = 106;}

                        else if ( ((LA42_104>='\u0000' && LA42_104<='\t')||(LA42_104>='\u000B' && LA42_104<='\f')||(LA42_104>='\u000E' && LA42_104<='\uFFFF')) ) {s = 104;}

                        else s = 107;

                        if ( s>=0 ) return s;
                        break;
                    case 2 : 
                        int LA42_96 = input.LA(1);

                        s = -1;
                        if ( (LA42_96=='\r') ) {s = 93;}

                        else if ( (LA42_96=='\n') ) {s = 94;}

                        else if ( ((LA42_96>='\u0000' && LA42_96<='\t')||(LA42_96>='\u000B' && LA42_96<='\f')||(LA42_96>='\u000E' && LA42_96<='\uFFFF')) ) {s = 96;}

                        else s = 95;

                        if ( s>=0 ) return s;
                        break;
                    case 3 : 
                        int LA42_15 = input.LA(1);

                        s = -1;
                        if ( (LA42_15=='i') ) {s = 39;}

                        else if ( (LA42_15=='e') ) {s = 40;}

                        else if ( (LA42_15=='d') ) {s = 41;}

                        else if ( (LA42_15=='t') ) {s = 42;}

                        else if ( ((LA42_15>='\u0000' && LA42_15<='c')||(LA42_15>='f' && LA42_15<='h')||(LA42_15>='j' && LA42_15<='s')||(LA42_15>='u' && LA42_15<='\uFFFF')) ) {s = 43;}

                        else s = 21;

                        if ( s>=0 ) return s;
                        break;
                    case 4 : 
                        int LA42_102 = input.LA(1);

                        s = -1;
                        if ( (LA42_102=='\r') ) {s = 121;}

                        else if ( (LA42_102=='\n') ) {s = 122;}

                        else if ( ((LA42_102>='\u0000' && LA42_102<='\t')||(LA42_102>='\u000B' && LA42_102<='\f')||(LA42_102>='\u000E' && LA42_102<='\uFFFF')) ) {s = 124;}

                        else s = 123;

                        if ( s>=0 ) return s;
                        break;
                    case 5 : 
                        int LA42_87 = input.LA(1);

                        s = -1;
                        if ( (LA42_87=='\r') ) {s = 109;}

                        else if ( (LA42_87=='\n') ) {s = 110;}

                        else if ( ((LA42_87>='\u0000' && LA42_87<='\t')||(LA42_87>='\u000B' && LA42_87<='\f')||(LA42_87>='\u000E' && LA42_87<='\uFFFF')) ) {s = 112;}

                        else s = 111;

                        if ( s>=0 ) return s;
                        break;
                    case 6 : 
                        int LA42_120 = input.LA(1);

                        s = -1;
                        if ( (LA42_120=='\r') ) {s = 117;}

                        else if ( (LA42_120=='\n') ) {s = 118;}

                        else if ( ((LA42_120>='\u0000' && LA42_120<='\t')||(LA42_120>='\u000B' && LA42_120<='\f')||(LA42_120>='\u000E' && LA42_120<='\uFFFF')) ) {s = 120;}

                        else s = 119;

                        if ( s>=0 ) return s;
                        break;
                    case 7 : 
                        int LA42_0 = input.LA(1);

                        s = -1;
                        if ( (LA42_0=='[') ) {s = 1;}

                        else if ( (LA42_0=='+') ) {s = 2;}

                        else if ( (LA42_0==']') ) {s = 3;}

                        else if ( (LA42_0=='=') ) {s = 4;}

                        else if ( (LA42_0=='{') ) {s = 5;}

                        else if ( (LA42_0=='.') ) {s = 6;}

                        else if ( (LA42_0=='~') ) {s = 7;}

                        else if ( (LA42_0=='}') ) {s = 8;}

                        else if ( (LA42_0=='_') ) {s = 9;}

                        else if ( (LA42_0=='$') ) {s = 10;}

                        else if ( (LA42_0=='/') ) {s = 11;}

                        else if ( (LA42_0=='(') ) {s = 12;}

                        else if ( (LA42_0==')') ) {s = 13;}

                        else if ( (LA42_0=='<') ) {s = 14;}

                        else if ( (LA42_0=='#') ) {s = 15;}

                        else if ( (LA42_0=='\"') ) {s = 16;}

                        else if ( ((LA42_0>=',' && LA42_0<='-')||(LA42_0>='0' && LA42_0<='9')||(LA42_0>='A' && LA42_0<='Z')||(LA42_0>='a' && LA42_0<='z')) ) {s = 17;}

                        else if ( (LA42_0=='\r') ) {s = 18;}

                        else if ( (LA42_0=='\n') ) {s = 19;}

                        else if ( (LA42_0=='\t'||LA42_0==' ') ) {s = 20;}

                        else if ( ((LA42_0>='\u0000' && LA42_0<='\b')||(LA42_0>='\u000B' && LA42_0<='\f')||(LA42_0>='\u000E' && LA42_0<='\u001F')||LA42_0=='!'||(LA42_0>='%' && LA42_0<='\'')||LA42_0=='*'||(LA42_0>=':' && LA42_0<=';')||(LA42_0>='>' && LA42_0<='@')||LA42_0=='\\'||LA42_0=='^'||LA42_0=='`'||LA42_0=='|'||(LA42_0>='\u007F' && LA42_0<='\uFFFF')) ) {s = 21;}

                        if ( s>=0 ) return s;
                        break;
                    case 8 : 
                        int LA42_66 = input.LA(1);

                        s = -1;
                        if ( ((LA42_66>='\u0000' && LA42_66<='\t')||(LA42_66>='\u000B' && LA42_66<='\f')||(LA42_66>='\u000E' && LA42_66<='\uFFFF')) ) {s = 77;}

                        else if ( (LA42_66=='\r') ) {s = 78;}

                        else if ( (LA42_66=='\n') ) {s = 79;}

                        else s = 80;

                        if ( s>=0 ) return s;
                        break;
                    case 9 : 
                        int LA42_108 = input.LA(1);

                        s = -1;
                        if ( ((LA42_108>='\u0000' && LA42_108<='\t')||(LA42_108>='\u000B' && LA42_108<='\f')||(LA42_108>='\u000E' && LA42_108<='\uFFFF')) ) {s = 126;}

                        else if ( (LA42_108=='\r') ) {s = 127;}

                        else if ( (LA42_108=='\n') ) {s = 128;}

                        else s = 129;

                        if ( s>=0 ) return s;
                        break;
                    case 10 : 
                        int LA42_132 = input.LA(1);

                        s = -1;
                        if ( ((LA42_132>='\u0000' && LA42_132<='\t')||(LA42_132>='\u000B' && LA42_132<='\f')||(LA42_132>='\u000E' && LA42_132<='\uFFFF')) ) {s = 133;}

                        else if ( (LA42_132=='\r') ) {s = 134;}

                        else if ( (LA42_132=='\n') ) {s = 135;}

                        else s = 136;

                        if ( s>=0 ) return s;
                        break;
                    case 11 : 
                        int LA42_124 = input.LA(1);

                        s = -1;
                        if ( (LA42_124=='\r') ) {s = 121;}

                        else if ( (LA42_124=='\n') ) {s = 122;}

                        else if ( ((LA42_124>='\u0000' && LA42_124<='\t')||(LA42_124>='\u000B' && LA42_124<='\f')||(LA42_124>='\u000E' && LA42_124<='\uFFFF')) ) {s = 124;}

                        else s = 123;

                        if ( s>=0 ) return s;
                        break;
                    case 12 : 
                        int LA42_16 = input.LA(1);

                        s = -1;
                        if ( ((LA42_16>='\u0000' && LA42_16<='\uFFFF')) ) {s = 44;}

                        else s = 21;

                        if ( s>=0 ) return s;
                        break;
                    case 13 : 
                        int LA42_89 = input.LA(1);

                        s = -1;
                        if ( (LA42_89=='\r') ) {s = 90;}

                        else if ( (LA42_89=='\n') ) {s = 91;}

                        else if ( ((LA42_89>='\u0000' && LA42_89<='\t')||(LA42_89>='\u000B' && LA42_89<='\f')||(LA42_89>='\u000E' && LA42_89<='\uFFFF')) ) {s = 89;}

                        else s = 92;

                        if ( s>=0 ) return s;
                        break;
                    case 14 : 
                        int LA42_81 = input.LA(1);

                        s = -1;
                        if ( (LA42_81=='\r') ) {s = 97;}

                        else if ( (LA42_81=='\n') ) {s = 98;}

                        else if ( ((LA42_81>='\u0000' && LA42_81<='\t')||(LA42_81>='\u000B' && LA42_81<='\f')||(LA42_81>='\u000E' && LA42_81<='\uFFFF')) ) {s = 100;}

                        else s = 99;

                        if ( s>=0 ) return s;
                        break;
                    case 15 : 
                        int LA42_100 = input.LA(1);

                        s = -1;
                        if ( (LA42_100=='\r') ) {s = 97;}

                        else if ( (LA42_100=='\n') ) {s = 98;}

                        else if ( ((LA42_100>='\u0000' && LA42_100<='\t')||(LA42_100>='\u000B' && LA42_100<='\f')||(LA42_100>='\u000E' && LA42_100<='\uFFFF')) ) {s = 100;}

                        else s = 99;

                        if ( s>=0 ) return s;
                        break;
                    case 16 : 
                        int LA42_75 = input.LA(1);

                        s = -1;
                        if ( ((LA42_75>='\u0000' && LA42_75<='\t')||(LA42_75>='\u000B' && LA42_75<='\f')||(LA42_75>='\u000E' && LA42_75<='\uFFFF')) ) {s = 89;}

                        else if ( (LA42_75=='\r') ) {s = 90;}

                        else if ( (LA42_75=='\n') ) {s = 91;}

                        else s = 92;

                        if ( s>=0 ) return s;
                        break;
                    case 17 : 
                        int LA42_112 = input.LA(1);

                        s = -1;
                        if ( (LA42_112=='\r') ) {s = 109;}

                        else if ( (LA42_112=='\n') ) {s = 110;}

                        else if ( ((LA42_112>='\u0000' && LA42_112<='\t')||(LA42_112>='\u000B' && LA42_112<='\f')||(LA42_112>='\u000E' && LA42_112<='\uFFFF')) ) {s = 112;}

                        else s = 111;

                        if ( s>=0 ) return s;
                        break;
                    case 18 : 
                        int LA42_88 = input.LA(1);

                        s = -1;
                        if ( (LA42_88=='\r') ) {s = 113;}

                        else if ( (LA42_88=='\n') ) {s = 114;}

                        else if ( ((LA42_88>='\u0000' && LA42_88<='\t')||(LA42_88>='\u000B' && LA42_88<='\f')||(LA42_88>='\u000E' && LA42_88<='\uFFFF')) ) {s = 116;}

                        else s = 115;

                        if ( s>=0 ) return s;
                        break;
                    case 19 : 
                        int LA42_133 = input.LA(1);

                        s = -1;
                        if ( (LA42_133=='\r') ) {s = 134;}

                        else if ( (LA42_133=='\n') ) {s = 135;}

                        else if ( ((LA42_133>='\u0000' && LA42_133<='\t')||(LA42_133>='\u000B' && LA42_133<='\f')||(LA42_133>='\u000E' && LA42_133<='\uFFFF')) ) {s = 133;}

                        else s = 136;

                        if ( s>=0 ) return s;
                        break;
                    case 20 : 
                        int LA42_126 = input.LA(1);

                        s = -1;
                        if ( (LA42_126=='\r') ) {s = 127;}

                        else if ( (LA42_126=='\n') ) {s = 128;}

                        else if ( ((LA42_126>='\u0000' && LA42_126<='\t')||(LA42_126>='\u000B' && LA42_126<='\f')||(LA42_126>='\u000E' && LA42_126<='\uFFFF')) ) {s = 126;}

                        else s = 129;

                        if ( s>=0 ) return s;
                        break;
                    case 21 : 
                        int LA42_101 = input.LA(1);

                        s = -1;
                        if ( (LA42_101=='\r') ) {s = 117;}

                        else if ( (LA42_101=='\n') ) {s = 118;}

                        else if ( ((LA42_101>='\u0000' && LA42_101<='\t')||(LA42_101>='\u000B' && LA42_101<='\f')||(LA42_101>='\u000E' && LA42_101<='\uFFFF')) ) {s = 120;}

                        else s = 119;

                        if ( s>=0 ) return s;
                        break;
                    case 22 : 
                        int LA42_116 = input.LA(1);

                        s = -1;
                        if ( (LA42_116=='\r') ) {s = 113;}

                        else if ( (LA42_116=='\n') ) {s = 114;}

                        else if ( ((LA42_116>='\u0000' && LA42_116<='\t')||(LA42_116>='\u000B' && LA42_116<='\f')||(LA42_116>='\u000E' && LA42_116<='\uFFFF')) ) {s = 116;}

                        else s = 115;

                        if ( s>=0 ) return s;
                        break;
                    case 23 : 
                        int LA42_85 = input.LA(1);

                        s = -1;
                        if ( ((LA42_85>='\u0000' && LA42_85<='\t')||(LA42_85>='\u000B' && LA42_85<='\f')||(LA42_85>='\u000E' && LA42_85<='\uFFFF')) ) {s = 104;}

                        else if ( (LA42_85=='\r') ) {s = 105;}

                        else if ( (LA42_85=='\n') ) {s = 106;}

                        else s = 107;

                        if ( s>=0 ) return s;
                        break;
                    case 24 : 
                        int LA42_77 = input.LA(1);

                        s = -1;
                        if ( (LA42_77=='\r') ) {s = 78;}

                        else if ( (LA42_77=='\n') ) {s = 79;}

                        else if ( ((LA42_77>='\u0000' && LA42_77<='\t')||(LA42_77>='\u000B' && LA42_77<='\f')||(LA42_77>='\u000E' && LA42_77<='\uFFFF')) ) {s = 77;}

                        else s = 80;

                        if ( s>=0 ) return s;
                        break;
            }
            NoViableAltException nvae =
                new NoViableAltException(getDescription(), 42, _s, input);
            error(nvae);
            throw nvae;
        }
    }
 

}