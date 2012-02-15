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
    public static final int RULE_LUA_CODE=16;
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
    public static final int T__32=32;
    public static final int RULE_STRING=17;
    public static final int T__33=33;
    public static final int T__34=34;
    public static final int RULE_ENDIF=14;
    public static final int RULE_DEFINE=7;
    public static final int RULE_ENDDEF=8;
    public static final int RULE_IFHAVE=11;
    public static final int RULE_WS=19;
    public static final int RULE_ELSE=13;

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

    // $ANTLR start "T__20"
    public final void mT__20() throws RecognitionException {
        try {
            int _type = T__20;
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
    // $ANTLR end "T__20"

    // $ANTLR start "T__21"
    public final void mT__21() throws RecognitionException {
        try {
            int _type = T__21;
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
    // $ANTLR end "T__21"

    // $ANTLR start "T__22"
    public final void mT__22() throws RecognitionException {
        try {
            int _type = T__22;
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
    // $ANTLR end "T__22"

    // $ANTLR start "T__23"
    public final void mT__23() throws RecognitionException {
        try {
            int _type = T__23;
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
    // $ANTLR end "T__23"

    // $ANTLR start "T__24"
    public final void mT__24() throws RecognitionException {
        try {
            int _type = T__24;
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
    // $ANTLR end "T__24"

    // $ANTLR start "T__25"
    public final void mT__25() throws RecognitionException {
        try {
            int _type = T__25;
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
    // $ANTLR end "T__25"

    // $ANTLR start "T__26"
    public final void mT__26() throws RecognitionException {
        try {
            int _type = T__26;
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
    // $ANTLR end "T__26"

    // $ANTLR start "T__27"
    public final void mT__27() throws RecognitionException {
        try {
            int _type = T__27;
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
    // $ANTLR end "T__27"

    // $ANTLR start "T__28"
    public final void mT__28() throws RecognitionException {
        try {
            int _type = T__28;
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
    // $ANTLR end "T__28"

    // $ANTLR start "T__29"
    public final void mT__29() throws RecognitionException {
        try {
            int _type = T__29;
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
    // $ANTLR end "T__29"

    // $ANTLR start "T__30"
    public final void mT__30() throws RecognitionException {
        try {
            int _type = T__30;
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
    // $ANTLR end "T__30"

    // $ANTLR start "T__31"
    public final void mT__31() throws RecognitionException {
        try {
            int _type = T__31;
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
    // $ANTLR end "T__31"

    // $ANTLR start "T__32"
    public final void mT__32() throws RecognitionException {
        try {
            int _type = T__32;
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
    // $ANTLR end "T__32"

    // $ANTLR start "T__33"
    public final void mT__33() throws RecognitionException {
        try {
            int _type = T__33;
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
    // $ANTLR end "T__33"

    // $ANTLR start "T__34"
    public final void mT__34() throws RecognitionException {
        try {
            int _type = T__34;
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
    // $ANTLR end "T__34"

    // $ANTLR start "RULE_LUA_CODE"
    public final void mRULE_LUA_CODE() throws RecognitionException {
        try {
            int _type = RULE_LUA_CODE;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:15: ( '<<' ( options {greedy=false; } : . )* '>>' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:17: '<<' ( options {greedy=false; } : . )* '>>'
            {
            match("<<"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:22: ( options {greedy=false; } : . )*
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:50: .
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:13: ( '#ifhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:15: '#ifhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifhave"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>='\u0000' && LA2_0<='\t')||(LA2_0>='\u000B' && LA2_0<='\f')||(LA2_0>='\u000E' && LA2_0<='\uFFFF')) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:25: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:41: ( ( '\\r' )? '\\n' )?
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0=='\n'||LA4_0=='\r') ) {
                alt4=1;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:42: ( '\\r' )?
                    int alt3=2;
                    int LA3_0 = input.LA(1);

                    if ( (LA3_0=='\r') ) {
                        alt3=1;
                    }
                    switch (alt3) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:42: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:14: ( '#ifnhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:16: '#ifnhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifnhave"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:27: (~ ( ( '\\n' | '\\r' ) ) )*
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( ((LA5_0>='\u0000' && LA5_0<='\t')||(LA5_0>='\u000B' && LA5_0<='\f')||(LA5_0>='\u000E' && LA5_0<='\uFFFF')) ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:27: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:43: ( ( '\\r' )? '\\n' )?
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0=='\n'||LA7_0=='\r') ) {
                alt7=1;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:44: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:44: ( '\\r' )?
                    int alt6=2;
                    int LA6_0 = input.LA(1);

                    if ( (LA6_0=='\r') ) {
                        alt6=1;
                    }
                    switch (alt6) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:44: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:12: ( '#ifdef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:14: '#ifdef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifdef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( ((LA8_0>='\u0000' && LA8_0<='\t')||(LA8_0>='\u000B' && LA8_0<='\f')||(LA8_0>='\u000E' && LA8_0<='\uFFFF')) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:23: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:39: ( ( '\\r' )? '\\n' )?
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0=='\n'||LA10_0=='\r') ) {
                alt10=1;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:40: ( '\\r' )?
                    int alt9=2;
                    int LA9_0 = input.LA(1);

                    if ( (LA9_0=='\r') ) {
                        alt9=1;
                    }
                    switch (alt9) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:40: '\\r'
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

    // $ANTLR start "RULE_IFNDEF"
    public final void mRULE_IFNDEF() throws RecognitionException {
        try {
            int _type = RULE_IFNDEF;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:13: ( '#ifndef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:15: '#ifndef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifndef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop11:
            do {
                int alt11=2;
                int LA11_0 = input.LA(1);

                if ( ((LA11_0>='\u0000' && LA11_0<='\t')||(LA11_0>='\u000B' && LA11_0<='\f')||(LA11_0>='\u000E' && LA11_0<='\uFFFF')) ) {
                    alt11=1;
                }


                switch (alt11) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:25: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:41: ( ( '\\r' )? '\\n' )?
            int alt13=2;
            int LA13_0 = input.LA(1);

            if ( (LA13_0=='\n'||LA13_0=='\r') ) {
                alt13=1;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:42: ( '\\r' )?
                    int alt12=2;
                    int LA12_0 = input.LA(1);

                    if ( (LA12_0=='\r') ) {
                        alt12=1;
                    }
                    switch (alt12) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:42: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:11: ( '#else' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:13: '#else' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#else"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:21: (~ ( ( '\\n' | '\\r' ) ) )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>='\u0000' && LA14_0<='\t')||(LA14_0>='\u000B' && LA14_0<='\f')||(LA14_0>='\u000E' && LA14_0<='\uFFFF')) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:21: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:37: ( ( '\\r' )? '\\n' )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0=='\n'||LA16_0=='\r') ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:38: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:38: ( '\\r' )?
                    int alt15=2;
                    int LA15_0 = input.LA(1);

                    if ( (LA15_0=='\r') ) {
                        alt15=1;
                    }
                    switch (alt15) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:38: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:12: ( '#endif' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:14: '#endif' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#endif"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop17:
            do {
                int alt17=2;
                int LA17_0 = input.LA(1);

                if ( ((LA17_0>='\u0000' && LA17_0<='\t')||(LA17_0>='\u000B' && LA17_0<='\f')||(LA17_0>='\u000E' && LA17_0<='\uFFFF')) ) {
                    alt17=1;
                }


                switch (alt17) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:23: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:39: ( ( '\\r' )? '\\n' )?
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0=='\n'||LA19_0=='\r') ) {
                alt19=1;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:40: ( '\\r' )?
                    int alt18=2;
                    int LA18_0 = input.LA(1);

                    if ( (LA18_0=='\r') ) {
                        alt18=1;
                    }
                    switch (alt18) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:40: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:13: ( '#define' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:15: '#define' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#define"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop20:
            do {
                int alt20=2;
                int LA20_0 = input.LA(1);

                if ( ((LA20_0>='\u0000' && LA20_0<='\t')||(LA20_0>='\u000B' && LA20_0<='\f')||(LA20_0>='\u000E' && LA20_0<='\uFFFF')) ) {
                    alt20=1;
                }


                switch (alt20) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:25: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:41: ( ( '\\r' )? '\\n' )?
            int alt22=2;
            int LA22_0 = input.LA(1);

            if ( (LA22_0=='\n'||LA22_0=='\r') ) {
                alt22=1;
            }
            switch (alt22) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:42: ( '\\r' )?
                    int alt21=2;
                    int LA21_0 = input.LA(1);

                    if ( (LA21_0=='\r') ) {
                        alt21=1;
                    }
                    switch (alt21) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:42: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:13: ( '#enddef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:15: '#enddef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#enddef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop23:
            do {
                int alt23=2;
                int LA23_0 = input.LA(1);

                if ( ((LA23_0>='\u0000' && LA23_0<='\t')||(LA23_0>='\u000B' && LA23_0<='\f')||(LA23_0>='\u000E' && LA23_0<='\uFFFF')) ) {
                    alt23=1;
                }


                switch (alt23) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:25: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:41: ( ( '\\r' )? '\\n' )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0=='\n'||LA25_0=='\r') ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:42: ( '\\r' )?
                    int alt24=2;
                    int LA24_0 = input.LA(1);

                    if ( (LA24_0=='\r') ) {
                        alt24=1;
                    }
                    switch (alt24) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:42: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:17: ( '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:19: '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#textdomain"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:33: (~ ( ( '\\n' | '\\r' ) ) )*
            loop26:
            do {
                int alt26=2;
                int LA26_0 = input.LA(1);

                if ( ((LA26_0>='\u0000' && LA26_0<='\t')||(LA26_0>='\u000B' && LA26_0<='\f')||(LA26_0>='\u000E' && LA26_0<='\uFFFF')) ) {
                    alt26=1;
                }


                switch (alt26) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:33: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:49: ( ( '\\r' )? '\\n' )?
            int alt28=2;
            int LA28_0 = input.LA(1);

            if ( (LA28_0=='\n'||LA28_0=='\r') ) {
                alt28=1;
            }
            switch (alt28) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:50: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:50: ( '\\r' )?
                    int alt27=2;
                    int LA27_0 = input.LA(1);

                    if ( (LA27_0=='\r') ) {
                        alt27=1;
                    }
                    switch (alt27) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:50: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:13: ( '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:15: '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"'
            {
            match('\"'); 
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:19: ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )*
            loop29:
            do {
                int alt29=3;
                int LA29_0 = input.LA(1);

                if ( (LA29_0=='\\') ) {
                    alt29=1;
                }
                else if ( ((LA29_0>='\u0000' && LA29_0<='!')||(LA29_0>='#' && LA29_0<='[')||(LA29_0>=']' && LA29_0<='\uFFFF')) ) {
                    alt29=2;
                }


                switch (alt29) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:20: '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' )
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:61: ~ ( ( '\\\\' | '\"' ) )
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
            	    break loop29;
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1371:9: ( ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+ )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1371:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1371:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+
            int cnt30=0;
            loop30:
            do {
                int alt30=2;
                int LA30_0 = input.LA(1);

                if ( ((LA30_0>=',' && LA30_0<='-')||(LA30_0>='0' && LA30_0<='9')||(LA30_0>='A' && LA30_0<='Z')||LA30_0=='_'||(LA30_0>='a' && LA30_0<='z')) ) {
                    alt30=1;
                }


                switch (alt30) {
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
            	    if ( cnt30 >= 1 ) break loop30;
                        EarlyExitException eee =
                            new EarlyExitException(30, input);
                        throw eee;
                }
                cnt30++;
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1373:10: ( ( '\\r' )? '\\n' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1373:12: ( '\\r' )? '\\n'
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1373:12: ( '\\r' )?
            int alt31=2;
            int LA31_0 = input.LA(1);

            if ( (LA31_0=='\r') ) {
                alt31=1;
            }
            switch (alt31) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1373:12: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1375:9: ( ( ' ' | '\\t' )+ )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1375:11: ( ' ' | '\\t' )+
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1375:11: ( ' ' | '\\t' )+
            int cnt32=0;
            loop32:
            do {
                int alt32=2;
                int LA32_0 = input.LA(1);

                if ( (LA32_0=='\t'||LA32_0==' ') ) {
                    alt32=1;
                }


                switch (alt32) {
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
            	    if ( cnt32 >= 1 ) break loop32;
                        EarlyExitException eee =
                            new EarlyExitException(32, input);
                        throw eee;
                }
                cnt32++;
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:16: ( . )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:18: .
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:17: ( '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:19: '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match('#'); 
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop33:
            do {
                int alt33=2;
                int LA33_0 = input.LA(1);

                if ( ((LA33_0>='\u0000' && LA33_0<='\t')||(LA33_0>='\u000B' && LA33_0<='\f')||(LA33_0>='\u000E' && LA33_0<='\uFFFF')) ) {
                    alt33=1;
                }


                switch (alt33) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:23: ~ ( ( '\\n' | '\\r' ) )
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
            	    break loop33;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:39: ( ( '\\r' )? '\\n' )?
            int alt35=2;
            int LA35_0 = input.LA(1);

            if ( (LA35_0=='\n'||LA35_0=='\r') ) {
                alt35=1;
            }
            switch (alt35) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:40: ( '\\r' )?
                    int alt34=2;
                    int LA34_0 = input.LA(1);

                    if ( (LA34_0=='\r') ) {
                        alt34=1;
                    }
                    switch (alt34) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1379:40: '\\r'
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
        // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:8: ( T__20 | T__21 | T__22 | T__23 | T__24 | T__25 | T__26 | T__27 | T__28 | T__29 | T__30 | T__31 | T__32 | T__33 | T__34 | RULE_LUA_CODE | RULE_IFHAVE | RULE_IFNHAVE | RULE_IFDEF | RULE_IFNDEF | RULE_ELSE | RULE_ENDIF | RULE_DEFINE | RULE_ENDDEF | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT )
        int alt36=31;
        alt36 = dfa36.predict(input);
        switch (alt36) {
            case 1 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:10: T__20
                {
                mT__20(); 

                }
                break;
            case 2 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:16: T__21
                {
                mT__21(); 

                }
                break;
            case 3 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:22: T__22
                {
                mT__22(); 

                }
                break;
            case 4 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:28: T__23
                {
                mT__23(); 

                }
                break;
            case 5 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:34: T__24
                {
                mT__24(); 

                }
                break;
            case 6 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:40: T__25
                {
                mT__25(); 

                }
                break;
            case 7 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:46: T__26
                {
                mT__26(); 

                }
                break;
            case 8 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:52: T__27
                {
                mT__27(); 

                }
                break;
            case 9 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:58: T__28
                {
                mT__28(); 

                }
                break;
            case 10 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:64: T__29
                {
                mT__29(); 

                }
                break;
            case 11 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:70: T__30
                {
                mT__30(); 

                }
                break;
            case 12 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:76: T__31
                {
                mT__31(); 

                }
                break;
            case 13 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:82: T__32
                {
                mT__32(); 

                }
                break;
            case 14 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:88: T__33
                {
                mT__33(); 

                }
                break;
            case 15 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:94: T__34
                {
                mT__34(); 

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
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:150: RULE_IFNDEF
                {
                mRULE_IFNDEF(); 

                }
                break;
            case 21 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:162: RULE_ELSE
                {
                mRULE_ELSE(); 

                }
                break;
            case 22 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:172: RULE_ENDIF
                {
                mRULE_ENDIF(); 

                }
                break;
            case 23 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:183: RULE_DEFINE
                {
                mRULE_DEFINE(); 

                }
                break;
            case 24 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:195: RULE_ENDDEF
                {
                mRULE_ENDDEF(); 

                }
                break;
            case 25 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:207: RULE_TEXTDOMAIN
                {
                mRULE_TEXTDOMAIN(); 

                }
                break;
            case 26 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:223: RULE_STRING
                {
                mRULE_STRING(); 

                }
                break;
            case 27 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:235: RULE_ID
                {
                mRULE_ID(); 

                }
                break;
            case 28 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:243: RULE_EOL
                {
                mRULE_EOL(); 

                }
                break;
            case 29 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:252: RULE_WS
                {
                mRULE_WS(); 

                }
                break;
            case 30 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:260: RULE_ANY_OTHER
                {
                mRULE_ANY_OTHER(); 

                }
                break;
            case 31 :
                // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1:275: RULE_SL_COMMENT
                {
                mRULE_SL_COMMENT(); 

                }
                break;

        }

    }


    protected DFA36 dfa36 = new DFA36(this);
    static final String DFA36_eotS =
        "\1\uffff\1\27\4\uffff\1\35\2\uffff\1\40\4\uffff\3\25\1\uffff\1"+
        "\25\24\uffff\4\53\4\uffff\20\53\1\113\7\53\1\126\1\113\3\uffff\1"+
        "\132\3\53\1\141\1\53\1\146\1\126\3\uffff\1\132\3\uffff\1\152\1\156"+
        "\1\53\1\141\3\uffff\1\163\1\146\3\uffff\1\152\3\uffff\1\156\3\uffff"+
        "\1\53\1\163\3\uffff\2\53\2\172\3\uffff";
    static final String DFA36_eofS =
        "\173\uffff";
    static final String DFA36_minS =
        "\1\0\1\57\4\uffff\1\57\2\uffff\1\54\4\uffff\1\74\2\0\1\uffff\1"+
        "\12\24\uffff\1\146\1\154\2\145\4\uffff\1\144\1\163\1\144\1\146\1"+
        "\170\1\141\1\144\2\145\1\144\1\151\1\164\1\166\1\141\1\145\1\146"+
        "\1\0\1\146\1\145\1\156\1\144\1\145\1\166\1\146\2\0\1\12\2\uffff"+
        "\1\0\1\146\1\145\1\157\1\0\1\145\2\0\1\12\2\uffff\1\0\1\12\2\uffff"+
        "\2\0\1\155\1\0\1\12\2\uffff\2\0\1\12\2\uffff\1\0\1\12\2\uffff\1"+
        "\0\1\12\2\uffff\1\141\1\0\1\12\2\uffff\1\151\1\156\2\0\1\12\2\uffff";
    static final String DFA36_maxS =
        "\1\uffff\1\57\4\uffff\1\57\2\uffff\1\172\4\uffff\1\74\2\uffff\1"+
        "\uffff\1\12\24\uffff\1\146\1\156\2\145\4\uffff\1\156\1\163\1\144"+
        "\1\146\1\170\1\141\1\150\2\145\2\151\1\164\1\166\1\141\1\145\1\146"+
        "\1\uffff\1\146\1\145\1\156\1\144\1\145\1\166\1\146\2\uffff\1\12"+
        "\2\uffff\1\uffff\1\146\1\145\1\157\1\uffff\1\145\2\uffff\1\12\2"+
        "\uffff\1\uffff\1\12\2\uffff\2\uffff\1\155\1\uffff\1\12\2\uffff\2"+
        "\uffff\1\12\2\uffff\1\uffff\1\12\2\uffff\1\uffff\1\12\2\uffff\1"+
        "\141\1\uffff\1\12\2\uffff\1\151\1\156\2\uffff\1\12\2\uffff";
    static final String DFA36_acceptS =
        "\2\uffff\1\2\1\3\1\5\1\6\1\uffff\1\10\1\11\1\uffff\1\14\1\15\1"+
        "\16\1\17\3\uffff\1\33\1\uffff\1\34\1\35\1\36\1\4\1\1\1\2\1\3\1\5"+
        "\1\6\1\7\1\13\1\10\1\11\1\12\1\33\1\14\1\15\1\16\1\17\1\20\4\uffff"+
        "\1\37\1\32\1\34\1\35\33\uffff\2\25\11\uffff\2\23\2\uffff\2\26\5"+
        "\uffff\2\21\3\uffff\2\24\2\uffff\2\30\2\uffff\2\27\3\uffff\2\22"+
        "\5\uffff\2\31";
    static final String DFA36_specialS =
        "\1\15\16\uffff\1\7\1\23\56\uffff\1\16\7\uffff\1\22\1\24\3\uffff"+
        "\1\3\3\uffff\1\4\1\uffff\1\6\1\5\3\uffff\1\14\3\uffff\1\13\1\12"+
        "\1\uffff\1\21\3\uffff\1\11\1\1\3\uffff\1\10\3\uffff\1\0\4\uffff"+
        "\1\2\5\uffff\1\20\1\17\3\uffff}>";
    static final String[] DFA36_transitionS = {
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
            "\1\66\3\uffff\1\64\5\uffff\1\65",
            "\1\67",
            "\1\70",
            "\1\71",
            "\1\72",
            "\1\73",
            "\1\75\3\uffff\1\74",
            "\1\76",
            "\1\77",
            "\1\101\4\uffff\1\100",
            "\1\102",
            "\1\103",
            "\1\104",
            "\1\105",
            "\1\106",
            "\1\107",
            "\12\110\1\112\2\110\1\111\ufff2\110",
            "\1\114",
            "\1\115",
            "\1\116",
            "\1\117",
            "\1\120",
            "\1\121",
            "\1\122",
            "\12\123\1\125\2\123\1\124\ufff2\123",
            "\12\110\1\112\2\110\1\111\ufff2\110",
            "\1\112",
            "",
            "",
            "\12\127\1\131\2\127\1\130\ufff2\127",
            "\1\133",
            "\1\134",
            "\1\135",
            "\12\136\1\140\2\136\1\137\ufff2\136",
            "\1\142",
            "\12\143\1\145\2\143\1\144\ufff2\143",
            "\12\123\1\125\2\123\1\124\ufff2\123",
            "\1\125",
            "",
            "",
            "\12\127\1\131\2\127\1\130\ufff2\127",
            "\1\131",
            "",
            "",
            "\12\147\1\151\2\147\1\150\ufff2\147",
            "\12\153\1\155\2\153\1\154\ufff2\153",
            "\1\157",
            "\12\136\1\140\2\136\1\137\ufff2\136",
            "\1\140",
            "",
            "",
            "\12\160\1\162\2\160\1\161\ufff2\160",
            "\12\143\1\145\2\143\1\144\ufff2\143",
            "\1\145",
            "",
            "",
            "\12\147\1\151\2\147\1\150\ufff2\147",
            "\1\151",
            "",
            "",
            "\12\153\1\155\2\153\1\154\ufff2\153",
            "\1\155",
            "",
            "",
            "\1\164",
            "\12\160\1\162\2\160\1\161\ufff2\160",
            "\1\162",
            "",
            "",
            "\1\165",
            "\1\166",
            "\12\167\1\171\2\167\1\170\ufff2\167",
            "\12\167\1\171\2\167\1\170\ufff2\167",
            "\1\171",
            "",
            ""
    };

    static final short[] DFA36_eot = DFA.unpackEncodedString(DFA36_eotS);
    static final short[] DFA36_eof = DFA.unpackEncodedString(DFA36_eofS);
    static final char[] DFA36_min = DFA.unpackEncodedStringToUnsignedChars(DFA36_minS);
    static final char[] DFA36_max = DFA.unpackEncodedStringToUnsignedChars(DFA36_maxS);
    static final short[] DFA36_accept = DFA.unpackEncodedString(DFA36_acceptS);
    static final short[] DFA36_special = DFA.unpackEncodedString(DFA36_specialS);
    static final short[][] DFA36_transition;

    static {
        int numStates = DFA36_transitionS.length;
        DFA36_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA36_transition[i] = DFA.unpackEncodedString(DFA36_transitionS[i]);
        }
    }

    class DFA36 extends DFA {

        public DFA36(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 36;
            this.eot = DFA36_eot;
            this.eof = DFA36_eof;
            this.min = DFA36_min;
            this.max = DFA36_max;
            this.accept = DFA36_accept;
            this.special = DFA36_special;
            this.transition = DFA36_transition;
        }
        public String getDescription() {
            return "1:1: Tokens : ( T__20 | T__21 | T__22 | T__23 | T__24 | T__25 | T__26 | T__27 | T__28 | T__29 | T__30 | T__31 | T__32 | T__33 | T__34 | RULE_LUA_CODE | RULE_IFHAVE | RULE_IFNHAVE | RULE_IFDEF | RULE_IFNDEF | RULE_ELSE | RULE_ENDIF | RULE_DEFINE | RULE_ENDDEF | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT );";
        }
        public int specialStateTransition(int s, IntStream _input) throws NoViableAltException {
            IntStream input = _input;
        	int _s = s;
            switch ( s ) {
                    case 0 : 
                        int LA36_107 = input.LA(1);

                        s = -1;
                        if ( (LA36_107=='\r') ) {s = 108;}

                        else if ( (LA36_107=='\n') ) {s = 109;}

                        else if ( ((LA36_107>='\u0000' && LA36_107<='\t')||(LA36_107>='\u000B' && LA36_107<='\f')||(LA36_107>='\u000E' && LA36_107<='\uFFFF')) ) {s = 107;}

                        else s = 110;

                        if ( s>=0 ) return s;
                        break;
                    case 1 : 
                        int LA36_99 = input.LA(1);

                        s = -1;
                        if ( (LA36_99=='\r') ) {s = 100;}

                        else if ( (LA36_99=='\n') ) {s = 101;}

                        else if ( ((LA36_99>='\u0000' && LA36_99<='\t')||(LA36_99>='\u000B' && LA36_99<='\f')||(LA36_99>='\u000E' && LA36_99<='\uFFFF')) ) {s = 99;}

                        else s = 102;

                        if ( s>=0 ) return s;
                        break;
                    case 2 : 
                        int LA36_112 = input.LA(1);

                        s = -1;
                        if ( (LA36_112=='\r') ) {s = 113;}

                        else if ( (LA36_112=='\n') ) {s = 114;}

                        else if ( ((LA36_112>='\u0000' && LA36_112<='\t')||(LA36_112>='\u000B' && LA36_112<='\f')||(LA36_112>='\u000E' && LA36_112<='\uFFFF')) ) {s = 112;}

                        else s = 115;

                        if ( s>=0 ) return s;
                        break;
                    case 3 : 
                        int LA36_76 = input.LA(1);

                        s = -1;
                        if ( ((LA36_76>='\u0000' && LA36_76<='\t')||(LA36_76>='\u000B' && LA36_76<='\f')||(LA36_76>='\u000E' && LA36_76<='\uFFFF')) ) {s = 87;}

                        else if ( (LA36_76=='\r') ) {s = 88;}

                        else if ( (LA36_76=='\n') ) {s = 89;}

                        else s = 90;

                        if ( s>=0 ) return s;
                        break;
                    case 4 : 
                        int LA36_80 = input.LA(1);

                        s = -1;
                        if ( ((LA36_80>='\u0000' && LA36_80<='\t')||(LA36_80>='\u000B' && LA36_80<='\f')||(LA36_80>='\u000E' && LA36_80<='\uFFFF')) ) {s = 94;}

                        else if ( (LA36_80=='\r') ) {s = 95;}

                        else if ( (LA36_80=='\n') ) {s = 96;}

                        else s = 97;

                        if ( s>=0 ) return s;
                        break;
                    case 5 : 
                        int LA36_83 = input.LA(1);

                        s = -1;
                        if ( (LA36_83=='\r') ) {s = 84;}

                        else if ( (LA36_83=='\n') ) {s = 85;}

                        else if ( ((LA36_83>='\u0000' && LA36_83<='\t')||(LA36_83>='\u000B' && LA36_83<='\f')||(LA36_83>='\u000E' && LA36_83<='\uFFFF')) ) {s = 83;}

                        else s = 86;

                        if ( s>=0 ) return s;
                        break;
                    case 6 : 
                        int LA36_82 = input.LA(1);

                        s = -1;
                        if ( ((LA36_82>='\u0000' && LA36_82<='\t')||(LA36_82>='\u000B' && LA36_82<='\f')||(LA36_82>='\u000E' && LA36_82<='\uFFFF')) ) {s = 99;}

                        else if ( (LA36_82=='\r') ) {s = 100;}

                        else if ( (LA36_82=='\n') ) {s = 101;}

                        else s = 102;

                        if ( s>=0 ) return s;
                        break;
                    case 7 : 
                        int LA36_15 = input.LA(1);

                        s = -1;
                        if ( (LA36_15=='i') ) {s = 39;}

                        else if ( (LA36_15=='e') ) {s = 40;}

                        else if ( (LA36_15=='d') ) {s = 41;}

                        else if ( (LA36_15=='t') ) {s = 42;}

                        else if ( ((LA36_15>='\u0000' && LA36_15<='c')||(LA36_15>='f' && LA36_15<='h')||(LA36_15>='j' && LA36_15<='s')||(LA36_15>='u' && LA36_15<='\uFFFF')) ) {s = 43;}

                        else s = 21;

                        if ( s>=0 ) return s;
                        break;
                    case 8 : 
                        int LA36_103 = input.LA(1);

                        s = -1;
                        if ( (LA36_103=='\r') ) {s = 104;}

                        else if ( (LA36_103=='\n') ) {s = 105;}

                        else if ( ((LA36_103>='\u0000' && LA36_103<='\t')||(LA36_103>='\u000B' && LA36_103<='\f')||(LA36_103>='\u000E' && LA36_103<='\uFFFF')) ) {s = 103;}

                        else s = 106;

                        if ( s>=0 ) return s;
                        break;
                    case 9 : 
                        int LA36_98 = input.LA(1);

                        s = -1;
                        if ( ((LA36_98>='\u0000' && LA36_98<='\t')||(LA36_98>='\u000B' && LA36_98<='\f')||(LA36_98>='\u000E' && LA36_98<='\uFFFF')) ) {s = 112;}

                        else if ( (LA36_98=='\r') ) {s = 113;}

                        else if ( (LA36_98=='\n') ) {s = 114;}

                        else s = 115;

                        if ( s>=0 ) return s;
                        break;
                    case 10 : 
                        int LA36_92 = input.LA(1);

                        s = -1;
                        if ( ((LA36_92>='\u0000' && LA36_92<='\t')||(LA36_92>='\u000B' && LA36_92<='\f')||(LA36_92>='\u000E' && LA36_92<='\uFFFF')) ) {s = 107;}

                        else if ( (LA36_92=='\r') ) {s = 108;}

                        else if ( (LA36_92=='\n') ) {s = 109;}

                        else s = 110;

                        if ( s>=0 ) return s;
                        break;
                    case 11 : 
                        int LA36_91 = input.LA(1);

                        s = -1;
                        if ( ((LA36_91>='\u0000' && LA36_91<='\t')||(LA36_91>='\u000B' && LA36_91<='\f')||(LA36_91>='\u000E' && LA36_91<='\uFFFF')) ) {s = 103;}

                        else if ( (LA36_91=='\r') ) {s = 104;}

                        else if ( (LA36_91=='\n') ) {s = 105;}

                        else s = 106;

                        if ( s>=0 ) return s;
                        break;
                    case 12 : 
                        int LA36_87 = input.LA(1);

                        s = -1;
                        if ( (LA36_87=='\r') ) {s = 88;}

                        else if ( (LA36_87=='\n') ) {s = 89;}

                        else if ( ((LA36_87>='\u0000' && LA36_87<='\t')||(LA36_87>='\u000B' && LA36_87<='\f')||(LA36_87>='\u000E' && LA36_87<='\uFFFF')) ) {s = 87;}

                        else s = 90;

                        if ( s>=0 ) return s;
                        break;
                    case 13 : 
                        int LA36_0 = input.LA(1);

                        s = -1;
                        if ( (LA36_0=='[') ) {s = 1;}

                        else if ( (LA36_0=='+') ) {s = 2;}

                        else if ( (LA36_0==']') ) {s = 3;}

                        else if ( (LA36_0=='=') ) {s = 4;}

                        else if ( (LA36_0=='{') ) {s = 5;}

                        else if ( (LA36_0=='.') ) {s = 6;}

                        else if ( (LA36_0=='~') ) {s = 7;}

                        else if ( (LA36_0=='}') ) {s = 8;}

                        else if ( (LA36_0=='_') ) {s = 9;}

                        else if ( (LA36_0=='$') ) {s = 10;}

                        else if ( (LA36_0=='/') ) {s = 11;}

                        else if ( (LA36_0=='(') ) {s = 12;}

                        else if ( (LA36_0==')') ) {s = 13;}

                        else if ( (LA36_0=='<') ) {s = 14;}

                        else if ( (LA36_0=='#') ) {s = 15;}

                        else if ( (LA36_0=='\"') ) {s = 16;}

                        else if ( ((LA36_0>=',' && LA36_0<='-')||(LA36_0>='0' && LA36_0<='9')||(LA36_0>='A' && LA36_0<='Z')||(LA36_0>='a' && LA36_0<='z')) ) {s = 17;}

                        else if ( (LA36_0=='\r') ) {s = 18;}

                        else if ( (LA36_0=='\n') ) {s = 19;}

                        else if ( (LA36_0=='\t'||LA36_0==' ') ) {s = 20;}

                        else if ( ((LA36_0>='\u0000' && LA36_0<='\b')||(LA36_0>='\u000B' && LA36_0<='\f')||(LA36_0>='\u000E' && LA36_0<='\u001F')||LA36_0=='!'||(LA36_0>='%' && LA36_0<='\'')||LA36_0=='*'||(LA36_0>=':' && LA36_0<=';')||(LA36_0>='>' && LA36_0<='@')||LA36_0=='\\'||LA36_0=='^'||LA36_0=='`'||LA36_0=='|'||(LA36_0>='\u007F' && LA36_0<='\uFFFF')) ) {s = 21;}

                        if ( s>=0 ) return s;
                        break;
                    case 14 : 
                        int LA36_63 = input.LA(1);

                        s = -1;
                        if ( ((LA36_63>='\u0000' && LA36_63<='\t')||(LA36_63>='\u000B' && LA36_63<='\f')||(LA36_63>='\u000E' && LA36_63<='\uFFFF')) ) {s = 72;}

                        else if ( (LA36_63=='\r') ) {s = 73;}

                        else if ( (LA36_63=='\n') ) {s = 74;}

                        else s = 75;

                        if ( s>=0 ) return s;
                        break;
                    case 15 : 
                        int LA36_119 = input.LA(1);

                        s = -1;
                        if ( (LA36_119=='\r') ) {s = 120;}

                        else if ( (LA36_119=='\n') ) {s = 121;}

                        else if ( ((LA36_119>='\u0000' && LA36_119<='\t')||(LA36_119>='\u000B' && LA36_119<='\f')||(LA36_119>='\u000E' && LA36_119<='\uFFFF')) ) {s = 119;}

                        else s = 122;

                        if ( s>=0 ) return s;
                        break;
                    case 16 : 
                        int LA36_118 = input.LA(1);

                        s = -1;
                        if ( ((LA36_118>='\u0000' && LA36_118<='\t')||(LA36_118>='\u000B' && LA36_118<='\f')||(LA36_118>='\u000E' && LA36_118<='\uFFFF')) ) {s = 119;}

                        else if ( (LA36_118=='\r') ) {s = 120;}

                        else if ( (LA36_118=='\n') ) {s = 121;}

                        else s = 122;

                        if ( s>=0 ) return s;
                        break;
                    case 17 : 
                        int LA36_94 = input.LA(1);

                        s = -1;
                        if ( (LA36_94=='\r') ) {s = 95;}

                        else if ( (LA36_94=='\n') ) {s = 96;}

                        else if ( ((LA36_94>='\u0000' && LA36_94<='\t')||(LA36_94>='\u000B' && LA36_94<='\f')||(LA36_94>='\u000E' && LA36_94<='\uFFFF')) ) {s = 94;}

                        else s = 97;

                        if ( s>=0 ) return s;
                        break;
                    case 18 : 
                        int LA36_71 = input.LA(1);

                        s = -1;
                        if ( ((LA36_71>='\u0000' && LA36_71<='\t')||(LA36_71>='\u000B' && LA36_71<='\f')||(LA36_71>='\u000E' && LA36_71<='\uFFFF')) ) {s = 83;}

                        else if ( (LA36_71=='\r') ) {s = 84;}

                        else if ( (LA36_71=='\n') ) {s = 85;}

                        else s = 86;

                        if ( s>=0 ) return s;
                        break;
                    case 19 : 
                        int LA36_16 = input.LA(1);

                        s = -1;
                        if ( ((LA36_16>='\u0000' && LA36_16<='\uFFFF')) ) {s = 44;}

                        else s = 21;

                        if ( s>=0 ) return s;
                        break;
                    case 20 : 
                        int LA36_72 = input.LA(1);

                        s = -1;
                        if ( (LA36_72=='\r') ) {s = 73;}

                        else if ( (LA36_72=='\n') ) {s = 74;}

                        else if ( ((LA36_72>='\u0000' && LA36_72<='\t')||(LA36_72>='\u000B' && LA36_72<='\f')||(LA36_72>='\u000E' && LA36_72<='\uFFFF')) ) {s = 72;}

                        else s = 75;

                        if ( s>=0 ) return s;
                        break;
            }
            NoViableAltException nvae =
                new NoViableAltException(getDescription(), 36, _s, input);
            error(nvae);
            throw nvae;
        }
    }
 

}