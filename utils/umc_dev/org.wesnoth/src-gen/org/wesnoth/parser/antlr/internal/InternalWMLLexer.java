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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1347:15: ( '<<' ( options {greedy=false; } : . )* '>>' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1347:17: '<<' ( options {greedy=false; } : . )* '>>'
            {
            match("<<"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1347:22: ( options {greedy=false; } : . )*
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1347:50: .
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:13: ( '#ifhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:15: '#ifhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifhave"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop2:
            do {
                int alt2=2;
                int LA2_0 = input.LA(1);

                if ( ((LA2_0>='\u0000' && LA2_0<='\t')||(LA2_0>='\u000B' && LA2_0<='\f')||(LA2_0>='\u000E' && LA2_0<='\uFFFF')) ) {
                    alt2=1;
                }


                switch (alt2) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:25: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:41: ( ( '\\r' )? '\\n' )?
            int alt4=2;
            int LA4_0 = input.LA(1);

            if ( (LA4_0=='\n'||LA4_0=='\r') ) {
                alt4=1;
            }
            switch (alt4) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:42: ( '\\r' )?
                    int alt3=2;
                    int LA3_0 = input.LA(1);

                    if ( (LA3_0=='\r') ) {
                        alt3=1;
                    }
                    switch (alt3) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1349:42: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:14: ( '#ifnhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:16: '#ifnhave' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifnhave"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:27: (~ ( ( '\\n' | '\\r' ) ) )*
            loop5:
            do {
                int alt5=2;
                int LA5_0 = input.LA(1);

                if ( ((LA5_0>='\u0000' && LA5_0<='\t')||(LA5_0>='\u000B' && LA5_0<='\f')||(LA5_0>='\u000E' && LA5_0<='\uFFFF')) ) {
                    alt5=1;
                }


                switch (alt5) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:27: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:43: ( ( '\\r' )? '\\n' )?
            int alt7=2;
            int LA7_0 = input.LA(1);

            if ( (LA7_0=='\n'||LA7_0=='\r') ) {
                alt7=1;
            }
            switch (alt7) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:44: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:44: ( '\\r' )?
                    int alt6=2;
                    int LA6_0 = input.LA(1);

                    if ( (LA6_0=='\r') ) {
                        alt6=1;
                    }
                    switch (alt6) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1351:44: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:12: ( '#ifdef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:14: '#ifdef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifdef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop8:
            do {
                int alt8=2;
                int LA8_0 = input.LA(1);

                if ( ((LA8_0>='\u0000' && LA8_0<='\t')||(LA8_0>='\u000B' && LA8_0<='\f')||(LA8_0>='\u000E' && LA8_0<='\uFFFF')) ) {
                    alt8=1;
                }


                switch (alt8) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:23: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:39: ( ( '\\r' )? '\\n' )?
            int alt10=2;
            int LA10_0 = input.LA(1);

            if ( (LA10_0=='\n'||LA10_0=='\r') ) {
                alt10=1;
            }
            switch (alt10) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:40: ( '\\r' )?
                    int alt9=2;
                    int LA9_0 = input.LA(1);

                    if ( (LA9_0=='\r') ) {
                        alt9=1;
                    }
                    switch (alt9) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1353:40: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:13: ( '#ifndef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:15: '#ifndef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#ifndef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop11:
            do {
                int alt11=2;
                int LA11_0 = input.LA(1);

                if ( ((LA11_0>='\u0000' && LA11_0<='\t')||(LA11_0>='\u000B' && LA11_0<='\f')||(LA11_0>='\u000E' && LA11_0<='\uFFFF')) ) {
                    alt11=1;
                }


                switch (alt11) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:25: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:41: ( ( '\\r' )? '\\n' )?
            int alt13=2;
            int LA13_0 = input.LA(1);

            if ( (LA13_0=='\n'||LA13_0=='\r') ) {
                alt13=1;
            }
            switch (alt13) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:42: ( '\\r' )?
                    int alt12=2;
                    int LA12_0 = input.LA(1);

                    if ( (LA12_0=='\r') ) {
                        alt12=1;
                    }
                    switch (alt12) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1355:42: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:11: ( '#else' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:13: '#else' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#else"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:21: (~ ( ( '\\n' | '\\r' ) ) )*
            loop14:
            do {
                int alt14=2;
                int LA14_0 = input.LA(1);

                if ( ((LA14_0>='\u0000' && LA14_0<='\t')||(LA14_0>='\u000B' && LA14_0<='\f')||(LA14_0>='\u000E' && LA14_0<='\uFFFF')) ) {
                    alt14=1;
                }


                switch (alt14) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:21: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:37: ( ( '\\r' )? '\\n' )?
            int alt16=2;
            int LA16_0 = input.LA(1);

            if ( (LA16_0=='\n'||LA16_0=='\r') ) {
                alt16=1;
            }
            switch (alt16) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:38: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:38: ( '\\r' )?
                    int alt15=2;
                    int LA15_0 = input.LA(1);

                    if ( (LA15_0=='\r') ) {
                        alt15=1;
                    }
                    switch (alt15) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1357:38: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:12: ( '#endif' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:14: '#endif' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#endif"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop17:
            do {
                int alt17=2;
                int LA17_0 = input.LA(1);

                if ( ((LA17_0>='\u0000' && LA17_0<='\t')||(LA17_0>='\u000B' && LA17_0<='\f')||(LA17_0>='\u000E' && LA17_0<='\uFFFF')) ) {
                    alt17=1;
                }


                switch (alt17) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:23: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:39: ( ( '\\r' )? '\\n' )?
            int alt19=2;
            int LA19_0 = input.LA(1);

            if ( (LA19_0=='\n'||LA19_0=='\r') ) {
                alt19=1;
            }
            switch (alt19) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:40: ( '\\r' )?
                    int alt18=2;
                    int LA18_0 = input.LA(1);

                    if ( (LA18_0=='\r') ) {
                        alt18=1;
                    }
                    switch (alt18) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1359:40: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:13: ( '#define' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:15: '#define' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#define"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop20:
            do {
                int alt20=2;
                int LA20_0 = input.LA(1);

                if ( ((LA20_0>='\u0000' && LA20_0<='\t')||(LA20_0>='\u000B' && LA20_0<='\f')||(LA20_0>='\u000E' && LA20_0<='\uFFFF')) ) {
                    alt20=1;
                }


                switch (alt20) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:25: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:41: ( ( '\\r' )? '\\n' )?
            int alt22=2;
            int LA22_0 = input.LA(1);

            if ( (LA22_0=='\n'||LA22_0=='\r') ) {
                alt22=1;
            }
            switch (alt22) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:42: ( '\\r' )?
                    int alt21=2;
                    int LA21_0 = input.LA(1);

                    if ( (LA21_0=='\r') ) {
                        alt21=1;
                    }
                    switch (alt21) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1361:42: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:13: ( '#enddef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:15: '#enddef' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#enddef"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:25: (~ ( ( '\\n' | '\\r' ) ) )*
            loop23:
            do {
                int alt23=2;
                int LA23_0 = input.LA(1);

                if ( ((LA23_0>='\u0000' && LA23_0<='\t')||(LA23_0>='\u000B' && LA23_0<='\f')||(LA23_0>='\u000E' && LA23_0<='\uFFFF')) ) {
                    alt23=1;
                }


                switch (alt23) {
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
            	    break loop23;
                }
            } while (true);

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:41: ( ( '\\r' )? '\\n' )?
            int alt25=2;
            int LA25_0 = input.LA(1);

            if ( (LA25_0=='\n'||LA25_0=='\r') ) {
                alt25=1;
            }
            switch (alt25) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:42: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1363:42: ( '\\r' )?
                    int alt24=2;
                    int LA24_0 = input.LA(1);

                    if ( (LA24_0=='\r') ) {
                        alt24=1;
                    }
                    switch (alt24) {
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
    // $ANTLR end "RULE_ENDDEF"

    // $ANTLR start "RULE_TEXTDOMAIN"
    public final void mRULE_TEXTDOMAIN() throws RecognitionException {
        try {
            int _type = RULE_TEXTDOMAIN;
            int _channel = DEFAULT_TOKEN_CHANNEL;
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:17: ( '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:19: '#textdomain' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match("#textdomain"); 

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:33: (~ ( ( '\\n' | '\\r' ) ) )*
            loop26:
            do {
                int alt26=2;
                int LA26_0 = input.LA(1);

                if ( ((LA26_0>='\u0000' && LA26_0<='\t')||(LA26_0>='\u000B' && LA26_0<='\f')||(LA26_0>='\u000E' && LA26_0<='\uFFFF')) ) {
                    alt26=1;
                }


                switch (alt26) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:33: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:49: ( ( '\\r' )? '\\n' )?
            int alt28=2;
            int LA28_0 = input.LA(1);

            if ( (LA28_0=='\n'||LA28_0=='\r') ) {
                alt28=1;
            }
            switch (alt28) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:50: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:50: ( '\\r' )?
                    int alt27=2;
                    int LA27_0 = input.LA(1);

                    if ( (LA27_0=='\r') ) {
                        alt27=1;
                    }
                    switch (alt27) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1365:50: '\\r'
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:13: ( '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"' )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:15: '\"' ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )* '\"'
            {
            match('\"'); 
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:19: ( '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' ) | ~ ( ( '\\\\' | '\"' ) ) )*
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:20: '\\\\' ( 'b' | 't' | 'n' | 'f' | 'r' | '\"' | '\\'' | '\\\\' )
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
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1367:61: ~ ( ( '\\\\' | '\"' ) )
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:9: ( ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+ )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1369:11: ( 'a' .. 'z' | 'A' .. 'Z' | '0' .. '9' | '_' | '-' | ',' )+
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1371:10: ( ( '\\r' | '\\n' ) )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1371:12: ( '\\r' | '\\n' )
            {
            if ( input.LA(1)=='\n'||input.LA(1)=='\r' ) {
                input.consume();

            }
            else {
                MismatchedSetException mse = new MismatchedSetException(null,input);
                recover(mse);
                throw mse;}


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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1373:9: ( ( ' ' | '\\t' )+ )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1373:11: ( ' ' | '\\t' )+
            {
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1373:11: ( ' ' | '\\t' )+
            int cnt31=0;
            loop31:
            do {
                int alt31=2;
                int LA31_0 = input.LA(1);

                if ( (LA31_0=='\t'||LA31_0==' ') ) {
                    alt31=1;
                }


                switch (alt31) {
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
            	    if ( cnt31 >= 1 ) break loop31;
                        EarlyExitException eee =
                            new EarlyExitException(31, input);
                        throw eee;
                }
                cnt31++;
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1375:16: ( . )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1375:18: .
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
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:17: ( '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )? )
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:19: '#' (~ ( ( '\\n' | '\\r' ) ) )* ( ( '\\r' )? '\\n' )?
            {
            match('#'); 
            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:23: (~ ( ( '\\n' | '\\r' ) ) )*
            loop32:
            do {
                int alt32=2;
                int LA32_0 = input.LA(1);

                if ( ((LA32_0>='\u0000' && LA32_0<='\t')||(LA32_0>='\u000B' && LA32_0<='\f')||(LA32_0>='\u000E' && LA32_0<='\uFFFF')) ) {
                    alt32=1;
                }


                switch (alt32) {
            	case 1 :
            	    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:23: ~ ( ( '\\n' | '\\r' ) )
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

            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:39: ( ( '\\r' )? '\\n' )?
            int alt34=2;
            int LA34_0 = input.LA(1);

            if ( (LA34_0=='\n'||LA34_0=='\r') ) {
                alt34=1;
            }
            switch (alt34) {
                case 1 :
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:40: ( '\\r' )? '\\n'
                    {
                    // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:40: ( '\\r' )?
                    int alt33=2;
                    int LA33_0 = input.LA(1);

                    if ( (LA33_0=='\r') ) {
                        alt33=1;
                    }
                    switch (alt33) {
                        case 1 :
                            // ../org.wesnoth/src-gen/org/wesnoth/parser/antlr/internal/InternalWML.g:1377:40: '\\r'
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
        int alt35=31;
        alt35 = dfa35.predict(input);
        switch (alt35) {
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


    protected DFA35 dfa35 = new DFA35(this);
    static final String DFA35_eotS =
        "\1\uffff\1\26\4\uffff\1\34\2\uffff\1\37\4\uffff\3\24\25\uffff\4"+
        "\52\4\uffff\20\52\1\111\7\52\1\124\3\uffff\1\111\1\130\3\52\1\137"+
        "\1\52\1\144\3\uffff\1\124\3\uffff\1\130\1\150\1\154\1\52\3\uffff"+
        "\1\137\1\161\3\uffff\1\144\3\uffff\1\150\3\uffff\1\154\1\52\3\uffff"+
        "\1\161\2\52\1\170\3\uffff\1\170";
    static final String DFA35_eofS =
        "\172\uffff";
    static final String DFA35_minS =
        "\1\0\1\57\4\uffff\1\57\2\uffff\1\54\4\uffff\1\74\2\0\25\uffff\1"+
        "\146\1\154\2\145\4\uffff\1\144\1\163\1\144\1\146\1\170\1\141\1\144"+
        "\2\145\1\144\1\151\1\164\1\166\1\141\1\145\1\146\1\0\1\146\1\145"+
        "\1\156\1\144\1\145\1\166\1\146\1\0\1\12\2\uffff\2\0\1\146\1\145"+
        "\1\157\1\0\1\145\1\0\1\12\2\uffff\1\0\1\12\2\uffff\3\0\1\155\1\12"+
        "\2\uffff\2\0\1\12\2\uffff\1\0\1\12\2\uffff\1\0\1\12\2\uffff\1\0"+
        "\1\141\1\12\2\uffff\1\0\1\151\1\156\1\0\1\12\2\uffff\1\0";
    static final String DFA35_maxS =
        "\1\uffff\1\57\4\uffff\1\57\2\uffff\1\172\4\uffff\1\74\2\uffff\25"+
        "\uffff\1\146\1\156\2\145\4\uffff\1\156\1\163\1\144\1\146\1\170\1"+
        "\141\1\150\2\145\2\151\1\164\1\166\1\141\1\145\1\146\1\uffff\1\146"+
        "\1\145\1\156\1\144\1\145\1\166\1\146\1\uffff\1\12\2\uffff\2\uffff"+
        "\1\146\1\145\1\157\1\uffff\1\145\1\uffff\1\12\2\uffff\1\uffff\1"+
        "\12\2\uffff\3\uffff\1\155\1\12\2\uffff\2\uffff\1\12\2\uffff\1\uffff"+
        "\1\12\2\uffff\1\uffff\1\12\2\uffff\1\uffff\1\141\1\12\2\uffff\1"+
        "\uffff\1\151\1\156\1\uffff\1\12\2\uffff\1\uffff";
    static final String DFA35_acceptS =
        "\2\uffff\1\2\1\3\1\5\1\6\1\uffff\1\10\1\11\1\uffff\1\14\1\15\1"+
        "\16\1\17\3\uffff\1\33\1\34\1\35\1\36\1\4\1\1\1\2\1\3\1\5\1\6\1\7"+
        "\1\13\1\10\1\11\1\12\1\33\1\14\1\15\1\16\1\17\1\20\4\uffff\1\37"+
        "\1\32\1\34\1\35\32\uffff\2\25\11\uffff\2\23\2\uffff\2\26\5\uffff"+
        "\2\21\3\uffff\2\24\2\uffff\2\30\2\uffff\2\27\3\uffff\2\22\5\uffff"+
        "\2\31\1\uffff";
    static final String DFA35_specialS =
        "\1\16\16\uffff\1\1\1\23\55\uffff\1\5\7\uffff\1\0\3\uffff\1\10\1"+
        "\21\3\uffff\1\17\1\uffff\1\14\3\uffff\1\2\3\uffff\1\22\1\3\1\6\4"+
        "\uffff\1\13\1\7\3\uffff\1\15\3\uffff\1\4\3\uffff\1\20\4\uffff\1"+
        "\24\2\uffff\1\11\3\uffff\1\12}>";
    static final String[] DFA35_transitionS = {
            "\11\24\1\23\1\22\2\24\1\22\22\24\1\23\1\24\1\20\1\17\1\12\3"+
            "\24\1\14\1\15\1\24\1\2\2\21\1\6\1\13\12\21\2\24\1\16\1\4\3\24"+
            "\32\21\1\1\1\24\1\3\1\24\1\11\1\24\32\21\1\5\1\24\1\10\1\7\uff81"+
            "\24",
            "\1\25",
            "",
            "",
            "",
            "",
            "\1\33",
            "",
            "",
            "\2\40\2\uffff\12\40\7\uffff\32\40\4\uffff\1\40\1\uffff\32"+
            "\40",
            "",
            "",
            "",
            "",
            "\1\45",
            "\144\52\1\50\1\47\3\52\1\46\12\52\1\51\uff8b\52",
            "\0\53",
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
            "",
            "\1\56",
            "\1\57\1\uffff\1\60",
            "\1\61",
            "\1\62",
            "",
            "",
            "",
            "",
            "\1\65\3\uffff\1\63\5\uffff\1\64",
            "\1\66",
            "\1\67",
            "\1\70",
            "\1\71",
            "\1\72",
            "\1\74\3\uffff\1\73",
            "\1\75",
            "\1\76",
            "\1\100\4\uffff\1\77",
            "\1\101",
            "\1\102",
            "\1\103",
            "\1\104",
            "\1\105",
            "\1\106",
            "\12\112\1\110\2\112\1\107\ufff2\112",
            "\1\113",
            "\1\114",
            "\1\115",
            "\1\116",
            "\1\117",
            "\1\120",
            "\1\121",
            "\12\125\1\123\2\125\1\122\ufff2\125",
            "\1\110",
            "",
            "",
            "\12\112\1\110\2\112\1\107\ufff2\112",
            "\12\131\1\127\2\131\1\126\ufff2\131",
            "\1\132",
            "\1\133",
            "\1\134",
            "\12\140\1\136\2\140\1\135\ufff2\140",
            "\1\141",
            "\12\145\1\143\2\145\1\142\ufff2\145",
            "\1\123",
            "",
            "",
            "\12\125\1\123\2\125\1\122\ufff2\125",
            "\1\127",
            "",
            "",
            "\12\131\1\127\2\131\1\126\ufff2\131",
            "\12\151\1\147\2\151\1\146\ufff2\151",
            "\12\155\1\153\2\155\1\152\ufff2\155",
            "\1\156",
            "\1\136",
            "",
            "",
            "\12\140\1\136\2\140\1\135\ufff2\140",
            "\12\162\1\160\2\162\1\157\ufff2\162",
            "\1\143",
            "",
            "",
            "\12\145\1\143\2\145\1\142\ufff2\145",
            "\1\147",
            "",
            "",
            "\12\151\1\147\2\151\1\146\ufff2\151",
            "\1\153",
            "",
            "",
            "\12\155\1\153\2\155\1\152\ufff2\155",
            "\1\163",
            "\1\160",
            "",
            "",
            "\12\162\1\160\2\162\1\157\ufff2\162",
            "\1\164",
            "\1\165",
            "\12\171\1\167\2\171\1\166\ufff2\171",
            "\1\167",
            "",
            "",
            "\12\171\1\167\2\171\1\166\ufff2\171"
    };

    static final short[] DFA35_eot = DFA.unpackEncodedString(DFA35_eotS);
    static final short[] DFA35_eof = DFA.unpackEncodedString(DFA35_eofS);
    static final char[] DFA35_min = DFA.unpackEncodedStringToUnsignedChars(DFA35_minS);
    static final char[] DFA35_max = DFA.unpackEncodedStringToUnsignedChars(DFA35_maxS);
    static final short[] DFA35_accept = DFA.unpackEncodedString(DFA35_acceptS);
    static final short[] DFA35_special = DFA.unpackEncodedString(DFA35_specialS);
    static final short[][] DFA35_transition;

    static {
        int numStates = DFA35_transitionS.length;
        DFA35_transition = new short[numStates][];
        for (int i=0; i<numStates; i++) {
            DFA35_transition[i] = DFA.unpackEncodedString(DFA35_transitionS[i]);
        }
    }

    class DFA35 extends DFA {

        public DFA35(BaseRecognizer recognizer) {
            this.recognizer = recognizer;
            this.decisionNumber = 35;
            this.eot = DFA35_eot;
            this.eof = DFA35_eof;
            this.min = DFA35_min;
            this.max = DFA35_max;
            this.accept = DFA35_accept;
            this.special = DFA35_special;
            this.transition = DFA35_transition;
        }
        public String getDescription() {
            return "1:1: Tokens : ( T__20 | T__21 | T__22 | T__23 | T__24 | T__25 | T__26 | T__27 | T__28 | T__29 | T__30 | T__31 | T__32 | T__33 | T__34 | RULE_LUA_CODE | RULE_IFHAVE | RULE_IFNHAVE | RULE_IFDEF | RULE_IFNDEF | RULE_ELSE | RULE_ENDIF | RULE_DEFINE | RULE_ENDDEF | RULE_TEXTDOMAIN | RULE_STRING | RULE_ID | RULE_EOL | RULE_WS | RULE_ANY_OTHER | RULE_SL_COMMENT );";
        }
        public int specialStateTransition(int s, IntStream _input) throws NoViableAltException {
            IntStream input = _input;
        	int _s = s;
            switch ( s ) {
                    case 0 : 
                        int LA35_70 = input.LA(1);

                        s = -1;
                        if ( (LA35_70=='\r') ) {s = 82;}

                        else if ( (LA35_70=='\n') ) {s = 83;}

                        else if ( ((LA35_70>='\u0000' && LA35_70<='\t')||(LA35_70>='\u000B' && LA35_70<='\f')||(LA35_70>='\u000E' && LA35_70<='\uFFFF')) ) {s = 85;}

                        else s = 84;

                        if ( s>=0 ) return s;
                        break;
                    case 1 : 
                        int LA35_15 = input.LA(1);

                        s = -1;
                        if ( (LA35_15=='i') ) {s = 38;}

                        else if ( (LA35_15=='e') ) {s = 39;}

                        else if ( (LA35_15=='d') ) {s = 40;}

                        else if ( (LA35_15=='t') ) {s = 41;}

                        else if ( ((LA35_15>='\u0000' && LA35_15<='c')||(LA35_15>='f' && LA35_15<='h')||(LA35_15>='j' && LA35_15<='s')||(LA35_15>='u' && LA35_15<='\uFFFF')) ) {s = 42;}

                        else s = 20;

                        if ( s>=0 ) return s;
                        break;
                    case 2 : 
                        int LA35_85 = input.LA(1);

                        s = -1;
                        if ( (LA35_85=='\r') ) {s = 82;}

                        else if ( (LA35_85=='\n') ) {s = 83;}

                        else if ( ((LA35_85>='\u0000' && LA35_85<='\t')||(LA35_85>='\u000B' && LA35_85<='\f')||(LA35_85>='\u000E' && LA35_85<='\uFFFF')) ) {s = 85;}

                        else s = 84;

                        if ( s>=0 ) return s;
                        break;
                    case 3 : 
                        int LA35_90 = input.LA(1);

                        s = -1;
                        if ( (LA35_90=='\r') ) {s = 102;}

                        else if ( (LA35_90=='\n') ) {s = 103;}

                        else if ( ((LA35_90>='\u0000' && LA35_90<='\t')||(LA35_90>='\u000B' && LA35_90<='\f')||(LA35_90>='\u000E' && LA35_90<='\uFFFF')) ) {s = 105;}

                        else s = 104;

                        if ( s>=0 ) return s;
                        break;
                    case 4 : 
                        int LA35_105 = input.LA(1);

                        s = -1;
                        if ( (LA35_105=='\r') ) {s = 102;}

                        else if ( (LA35_105=='\n') ) {s = 103;}

                        else if ( ((LA35_105>='\u0000' && LA35_105<='\t')||(LA35_105>='\u000B' && LA35_105<='\f')||(LA35_105>='\u000E' && LA35_105<='\uFFFF')) ) {s = 105;}

                        else s = 104;

                        if ( s>=0 ) return s;
                        break;
                    case 5 : 
                        int LA35_62 = input.LA(1);

                        s = -1;
                        if ( (LA35_62=='\r') ) {s = 71;}

                        else if ( (LA35_62=='\n') ) {s = 72;}

                        else if ( ((LA35_62>='\u0000' && LA35_62<='\t')||(LA35_62>='\u000B' && LA35_62<='\f')||(LA35_62>='\u000E' && LA35_62<='\uFFFF')) ) {s = 74;}

                        else s = 73;

                        if ( s>=0 ) return s;
                        break;
                    case 6 : 
                        int LA35_91 = input.LA(1);

                        s = -1;
                        if ( (LA35_91=='\r') ) {s = 106;}

                        else if ( (LA35_91=='\n') ) {s = 107;}

                        else if ( ((LA35_91>='\u0000' && LA35_91<='\t')||(LA35_91>='\u000B' && LA35_91<='\f')||(LA35_91>='\u000E' && LA35_91<='\uFFFF')) ) {s = 109;}

                        else s = 108;

                        if ( s>=0 ) return s;
                        break;
                    case 7 : 
                        int LA35_97 = input.LA(1);

                        s = -1;
                        if ( (LA35_97=='\r') ) {s = 111;}

                        else if ( (LA35_97=='\n') ) {s = 112;}

                        else if ( ((LA35_97>='\u0000' && LA35_97<='\t')||(LA35_97>='\u000B' && LA35_97<='\f')||(LA35_97>='\u000E' && LA35_97<='\uFFFF')) ) {s = 114;}

                        else s = 113;

                        if ( s>=0 ) return s;
                        break;
                    case 8 : 
                        int LA35_74 = input.LA(1);

                        s = -1;
                        if ( (LA35_74=='\r') ) {s = 71;}

                        else if ( (LA35_74=='\n') ) {s = 72;}

                        else if ( ((LA35_74>='\u0000' && LA35_74<='\t')||(LA35_74>='\u000B' && LA35_74<='\f')||(LA35_74>='\u000E' && LA35_74<='\uFFFF')) ) {s = 74;}

                        else s = 73;

                        if ( s>=0 ) return s;
                        break;
                    case 9 : 
                        int LA35_117 = input.LA(1);

                        s = -1;
                        if ( (LA35_117=='\r') ) {s = 118;}

                        else if ( (LA35_117=='\n') ) {s = 119;}

                        else if ( ((LA35_117>='\u0000' && LA35_117<='\t')||(LA35_117>='\u000B' && LA35_117<='\f')||(LA35_117>='\u000E' && LA35_117<='\uFFFF')) ) {s = 121;}

                        else s = 120;

                        if ( s>=0 ) return s;
                        break;
                    case 10 : 
                        int LA35_121 = input.LA(1);

                        s = -1;
                        if ( (LA35_121=='\r') ) {s = 118;}

                        else if ( (LA35_121=='\n') ) {s = 119;}

                        else if ( ((LA35_121>='\u0000' && LA35_121<='\t')||(LA35_121>='\u000B' && LA35_121<='\f')||(LA35_121>='\u000E' && LA35_121<='\uFFFF')) ) {s = 121;}

                        else s = 120;

                        if ( s>=0 ) return s;
                        break;
                    case 11 : 
                        int LA35_96 = input.LA(1);

                        s = -1;
                        if ( (LA35_96=='\r') ) {s = 93;}

                        else if ( (LA35_96=='\n') ) {s = 94;}

                        else if ( ((LA35_96>='\u0000' && LA35_96<='\t')||(LA35_96>='\u000B' && LA35_96<='\f')||(LA35_96>='\u000E' && LA35_96<='\uFFFF')) ) {s = 96;}

                        else s = 95;

                        if ( s>=0 ) return s;
                        break;
                    case 12 : 
                        int LA35_81 = input.LA(1);

                        s = -1;
                        if ( (LA35_81=='\r') ) {s = 98;}

                        else if ( (LA35_81=='\n') ) {s = 99;}

                        else if ( ((LA35_81>='\u0000' && LA35_81<='\t')||(LA35_81>='\u000B' && LA35_81<='\f')||(LA35_81>='\u000E' && LA35_81<='\uFFFF')) ) {s = 101;}

                        else s = 100;

                        if ( s>=0 ) return s;
                        break;
                    case 13 : 
                        int LA35_101 = input.LA(1);

                        s = -1;
                        if ( (LA35_101=='\r') ) {s = 98;}

                        else if ( (LA35_101=='\n') ) {s = 99;}

                        else if ( ((LA35_101>='\u0000' && LA35_101<='\t')||(LA35_101>='\u000B' && LA35_101<='\f')||(LA35_101>='\u000E' && LA35_101<='\uFFFF')) ) {s = 101;}

                        else s = 100;

                        if ( s>=0 ) return s;
                        break;
                    case 14 : 
                        int LA35_0 = input.LA(1);

                        s = -1;
                        if ( (LA35_0=='[') ) {s = 1;}

                        else if ( (LA35_0=='+') ) {s = 2;}

                        else if ( (LA35_0==']') ) {s = 3;}

                        else if ( (LA35_0=='=') ) {s = 4;}

                        else if ( (LA35_0=='{') ) {s = 5;}

                        else if ( (LA35_0=='.') ) {s = 6;}

                        else if ( (LA35_0=='~') ) {s = 7;}

                        else if ( (LA35_0=='}') ) {s = 8;}

                        else if ( (LA35_0=='_') ) {s = 9;}

                        else if ( (LA35_0=='$') ) {s = 10;}

                        else if ( (LA35_0=='/') ) {s = 11;}

                        else if ( (LA35_0=='(') ) {s = 12;}

                        else if ( (LA35_0==')') ) {s = 13;}

                        else if ( (LA35_0=='<') ) {s = 14;}

                        else if ( (LA35_0=='#') ) {s = 15;}

                        else if ( (LA35_0=='\"') ) {s = 16;}

                        else if ( ((LA35_0>=',' && LA35_0<='-')||(LA35_0>='0' && LA35_0<='9')||(LA35_0>='A' && LA35_0<='Z')||(LA35_0>='a' && LA35_0<='z')) ) {s = 17;}

                        else if ( (LA35_0=='\n'||LA35_0=='\r') ) {s = 18;}

                        else if ( (LA35_0=='\t'||LA35_0==' ') ) {s = 19;}

                        else if ( ((LA35_0>='\u0000' && LA35_0<='\b')||(LA35_0>='\u000B' && LA35_0<='\f')||(LA35_0>='\u000E' && LA35_0<='\u001F')||LA35_0=='!'||(LA35_0>='%' && LA35_0<='\'')||LA35_0=='*'||(LA35_0>=':' && LA35_0<=';')||(LA35_0>='>' && LA35_0<='@')||LA35_0=='\\'||LA35_0=='^'||LA35_0=='`'||LA35_0=='|'||(LA35_0>='\u007F' && LA35_0<='\uFFFF')) ) {s = 20;}

                        if ( s>=0 ) return s;
                        break;
                    case 15 : 
                        int LA35_79 = input.LA(1);

                        s = -1;
                        if ( (LA35_79=='\r') ) {s = 93;}

                        else if ( (LA35_79=='\n') ) {s = 94;}

                        else if ( ((LA35_79>='\u0000' && LA35_79<='\t')||(LA35_79>='\u000B' && LA35_79<='\f')||(LA35_79>='\u000E' && LA35_79<='\uFFFF')) ) {s = 96;}

                        else s = 95;

                        if ( s>=0 ) return s;
                        break;
                    case 16 : 
                        int LA35_109 = input.LA(1);

                        s = -1;
                        if ( (LA35_109=='\r') ) {s = 106;}

                        else if ( (LA35_109=='\n') ) {s = 107;}

                        else if ( ((LA35_109>='\u0000' && LA35_109<='\t')||(LA35_109>='\u000B' && LA35_109<='\f')||(LA35_109>='\u000E' && LA35_109<='\uFFFF')) ) {s = 109;}

                        else s = 108;

                        if ( s>=0 ) return s;
                        break;
                    case 17 : 
                        int LA35_75 = input.LA(1);

                        s = -1;
                        if ( (LA35_75=='\r') ) {s = 86;}

                        else if ( (LA35_75=='\n') ) {s = 87;}

                        else if ( ((LA35_75>='\u0000' && LA35_75<='\t')||(LA35_75>='\u000B' && LA35_75<='\f')||(LA35_75>='\u000E' && LA35_75<='\uFFFF')) ) {s = 89;}

                        else s = 88;

                        if ( s>=0 ) return s;
                        break;
                    case 18 : 
                        int LA35_89 = input.LA(1);

                        s = -1;
                        if ( (LA35_89=='\r') ) {s = 86;}

                        else if ( (LA35_89=='\n') ) {s = 87;}

                        else if ( ((LA35_89>='\u0000' && LA35_89<='\t')||(LA35_89>='\u000B' && LA35_89<='\f')||(LA35_89>='\u000E' && LA35_89<='\uFFFF')) ) {s = 89;}

                        else s = 88;

                        if ( s>=0 ) return s;
                        break;
                    case 19 : 
                        int LA35_16 = input.LA(1);

                        s = -1;
                        if ( ((LA35_16>='\u0000' && LA35_16<='\uFFFF')) ) {s = 43;}

                        else s = 20;

                        if ( s>=0 ) return s;
                        break;
                    case 20 : 
                        int LA35_114 = input.LA(1);

                        s = -1;
                        if ( (LA35_114=='\r') ) {s = 111;}

                        else if ( (LA35_114=='\n') ) {s = 112;}

                        else if ( ((LA35_114>='\u0000' && LA35_114<='\t')||(LA35_114>='\u000B' && LA35_114<='\f')||(LA35_114>='\u000E' && LA35_114<='\uFFFF')) ) {s = 114;}

                        else s = 113;

                        if ( s>=0 ) return s;
                        break;
            }
            NoViableAltException nvae =
                new NoViableAltException(getDescription(), 35, _s, input);
            error(nvae);
            throw nvae;
        }
    }
 

}